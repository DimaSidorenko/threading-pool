#ifndef THREADPOOL_H
#define THREADPOOL_H


#include <atomic>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <type_traits>
#include <vector>
#include <iostream>
#include <string>
#include <list>
#include <unordered_map>
#include <map>
#include "taskinfo.h"


class thread_pool
{
private:
    //size_t tasks_counter;

    std::condition_variable event_obj;
    std::mutex lock_mutex;
    std::atomic<bool> is_thread_pool_in_destruction;

    // fixed-size threads array
    std::vector<std::thread> threads;

    // queue of tasks with elements {task, task_id}
    std::list<std::pair<std::function<std::string()>, size_t>> tasks_queue;

    // map with tasks status
    std::map<size_t, TaskStatus> tasks_status;

    // map with tasks results
    std::map<size_t, std::string> tasks_result;

    // init thread pool
    explicit thread_pool() {
        is_thread_pool_in_destruction = false;
    };

    // thread consume function
    static void task_consumer(thread_pool* pool)
    {
        // infinity consume and execute tasks
        while (true)
        {
            std::function<std::string()> task = []() {return ""; }; // create empty task
            std::size_t task_id = 0;

            {
                std::unique_lock<std::mutex> locker(pool->lock_mutex); // lock guard

                // wait event after inserting to queue
                pool->event_obj.wait(locker, [pool]()
                {
                    // if queue have items or pool is in destruction we exit from wait state
                    return pool->is_thread_pool_in_destruction || !pool->tasks_queue.empty();
                });

                // if pool is in destruction leave end current thread
                if (pool->is_thread_pool_in_destruction){
                    return;
                }

                // if we have items in queue, pop task

                auto front_element = pool->tasks_queue.begin();

                task = front_element->first;
                task_id = front_element->second;

                pool->tasks_queue.erase(front_element);
            }

            if (task_id != 0) {
                pool->task_started(task_id, std::this_thread::get_id());

                std::string result = task(); // execute task

                pool->task_finished(task_id, result);
            }
            else {
                std::cout << "task_id == 0, так не должно быть";
            }
        }
    }

    /**
    * \action of start task
    */
    void task_started(size_t task_id, std::thread::id thread_id) {
        std::unique_lock<std::mutex> lock(lock_mutex);

        //std::cout << "task with taskID = " << task_id << " started by thread = " << thread_id << '\n';

        tasks_status[task_id] = TaskStatus::in_progress;
    }

    /**
     * \action of finish task
     */
    void task_finished(size_t task_id, std::string result) {
        {
            std::unique_lock<std::mutex> lock(lock_mutex);
            tasks_result[task_id] = result;
            tasks_status[task_id] = TaskStatus::done;

        }

        //std::cout << "task with taskId = " << task_id << " finished with the result = " << result << '\n'
        event_obj.notify_one();
    }

public:

    // destruct thread pool
    ~thread_pool()
    {
        {
            std::unique_lock<std::mutex> locker(this->lock_mutex); // lock pool
            this->is_thread_pool_in_destruction = true; // set flag

            this->tasks_queue.clear(); // remove all tasks
        }

        this->event_obj.notify_all(); // notify all thread about closing

        for (std::thread& th : this->threads)
            if (th.joinable())
                th.join(); // bind eof of threads to main thread
     }


    TaskStatus get_task_status(size_t task_id) {
        std::unique_lock<std::mutex> locker(this->lock_mutex);

        if (tasks_status.count(task_id) == 0) {
            return TaskStatus::waiting;
        }

        return  tasks_status[task_id];
    }


    std::string get_task_result(size_t task_id) {
        std::unique_lock<std::mutex> locker(this->lock_mutex);

        if (tasks_result.count(task_id) == 0) {
            return "---";
        }

        return tasks_result[task_id];
    }



    //TO-DO rewrite this, there are 2 cases we can simply add or pop threads
    void start_pool(const size_t threadscount) {
        this->threads.resize(threadscount); // create array of threads
        this->is_thread_pool_in_destruction = false; // set flag


        for (std::thread& th : this->threads)
            th = std::thread(thread_pool::task_consumer, this); // init every thread

        event_obj.notify_one();
    }


    void stop_pool() {
        {
            std::unique_lock<std::mutex> locker(this->lock_mutex); // lock pool
            this->is_thread_pool_in_destruction = true; // set flag

            //std::cout << "stop_pool" << '\n';

            for (auto it = tasks_queue.begin(); it != tasks_queue.end(); it++) {
                tasks_result.erase((*it).second);
                tasks_status.erase((*it).second);
            }

            tasks_queue.clear(); // remove all task
          }

        this->event_obj.notify_all(); // notify all thread about closing

        for (std::thread& th : this->threads)
            if (th.joinable())
                th.join(); // bind eof of threads to main thread

        threads.clear();

    }


    void erase_task(size_t task_id) {
        std::unique_lock<std::mutex> locker(this->lock_mutex);

        for (auto it = tasks_queue.begin(); it != tasks_queue.end(); it++) {
            if ((*it).second == task_id) {
                tasks_queue.erase(it);
                break;
            }
        }
    }

    /**
     * \return thread pool instance
     */
    static thread_pool& instance()
    {
        static thread_pool pool; // classic singleton
        return pool;
    }

    /**
     * \return thread count in pool
     */
    size_t thread_capacity()
    {
        std::unique_lock<std::mutex> locker(this->lock_mutex); // lock guard

        return this->threads.size();
    }

    /**
     * \return queue task count
     */
    size_t thread_queue_task_count()
    {
        std::unique_lock<std::mutex> locker(this->lock_mutex); // lock guard

        return this->tasks_queue.size();
    }


    void add_task(std::string(*func)(int64_t), int64_t arg, size_t task_id) {
        std::function<std::string()> packed_task = ([=]
        {
            std::string result = func(arg);
            return result;
        });

        {
            std::unique_lock<std::mutex> lock(lock_mutex);

            tasks_queue.emplace_back(std::make_pair(packed_task, task_id));

            tasks_status[task_id] = TaskStatus::waiting;
        }
    }

    ///**
    // * \add task in tasks queue
    // */
    //template<typename F, class... Args>
    //void add_task(F&& func, Args&&... args)
    //{
    //	using return_type = std::invoke_result_t<F, Args...>;

    //	// create task
    //	auto task = std::make_shared<std::packaged_task<return_type()>>(std::bind(std::forward<F>(func), std::forward<Args>(args)...));

    //	// get future result
    //	std::future<return_type> res = task->get_future();

    //	std::function<void()> packed_task = ([=]
    //	{
    //		(*task)();
    //	});

    //	{
    //		std::unique_lock lock(lock_mutex);

    //		tasks_queue.emplace(std::make_pair(packed_task, ++tasks_counter));
    //
    //		tasks_status[tasks_counter] = TaskStatus::waiting;
    //	}

    //	event_obj.notify_one();
    //}
};

#endif // THREADPOOL_H
