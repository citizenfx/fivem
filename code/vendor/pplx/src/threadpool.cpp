/***
* Copyright (C) Microsoft. All rights reserved.
* Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
*
* =+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
**/
#include "stdafx.h"

#if !defined(CPPREST_EXCLUDE_WEBSOCKETS) || !defined(_WIN32)
#include "pplx/threadpool.h"

#if !defined(_WIN32)
#define CPPREST_PTHREADS
#endif

#if defined(CPPREST_PTHREADS)
#include <pthread.h>
#else
#include <thread>
#endif

#include <vector>

#if defined(__ANDROID__)
#include <android/log.h>
#include <jni.h>
#endif

namespace
{

struct threadpool_impl final : crossplat::threadpool
{
    threadpool_impl(size_t n)
        : crossplat::threadpool(n)
        , m_work(m_service)
    {
        for (size_t i = 0; i < n; i++)
            add_thread();
    }

    ~threadpool_impl()
    {
        m_service.stop();
        for (auto iter = m_threads.begin(); iter != m_threads.end(); ++iter)
        {
#if defined(CPPREST_PTHREADS)
            pthread_t t = *iter;
            void* res;
            pthread_join(t, &res);
#else
            iter->join();
#endif
        }
    }

private:
    void add_thread()
    {
#ifdef CPPREST_PTHREADS
        pthread_t t;
        auto result = pthread_create(&t, nullptr, &thread_start, this);
        if (result == 0)
            m_threads.push_back(t);
#else
        m_threads.push_back(std::thread(&thread_start, this));
#endif
    }

#if defined(__ANDROID__)
    static void detach_from_java(void*)
    {
        crossplat::JVM.load()->DetachCurrentThread();
    }
#endif

    static void* thread_start(void *arg) CPPREST_NOEXCEPT
    {
#if defined(__ANDROID__)
        // Calling get_jvm_env() here forces the thread to be attached.
        crossplat::get_jvm_env();
        pthread_cleanup_push(detach_from_java, nullptr);
#endif
        threadpool_impl* _this = reinterpret_cast<threadpool_impl*>(arg);
        _this->m_service.run();
#if defined(__ANDROID__)
        pthread_cleanup_pop(true);
#endif
        return arg;
    }

#if defined(CPPREST_PTHREADS)
    std::vector<pthread_t> m_threads;
#else
    std::vector<std::thread> m_threads;
#endif
    boost::asio::io_service::work m_work;
};
}

namespace crossplat
{
#if defined(__ANDROID__)
// This pointer will be 0-initialized by default (at load time).
std::atomic<JavaVM*> JVM;

static void abort_if_no_jvm()
{
    if (JVM == nullptr)
    {
        __android_log_print(ANDROID_LOG_ERROR, "CPPRESTSDK", "%s", "The CppREST SDK must be initialized before first use on android: https://github.com/Microsoft/cpprestsdk/wiki/How-to-build-for-Android");
        std::abort();
    }
}

JNIEnv* get_jvm_env()
{
    abort_if_no_jvm();
    JNIEnv* env = nullptr;
    auto result = JVM.load()->AttachCurrentThread(&env, nullptr);
    if (result != JNI_OK)
    {
        throw std::runtime_error("Could not attach to JVM");
    }

    return env;
}

threadpool& threadpool::shared_instance()
{
    abort_if_no_jvm();
    static threadpool_impl s_shared(40);
    return s_shared;
}

#else

// initialize the static shared threadpool
threadpool& threadpool::shared_instance()
{
    static threadpool_impl s_shared(40);
    return s_shared;
}

#endif

}

#if defined(__ANDROID__)
void cpprest_init(JavaVM* vm) {
    crossplat::JVM = vm;
}
#endif

std::unique_ptr<crossplat::threadpool> crossplat::threadpool::construct(size_t num_threads)
{
    return std::unique_ptr<crossplat::threadpool>(new threadpool_impl(num_threads));
}
#endif