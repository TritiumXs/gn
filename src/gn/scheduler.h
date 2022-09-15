// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TOOLS_GN_SCHEDULER_H_
#define TOOLS_GN_SCHEDULER_H_

#include <condition_variable>
#include <functional>
#include <map>
#include <mutex>

#include "base/atomic_ref_count.h"
#include "base/files/file_path.h"
#include "gn/input_file_manager.h"
#include "gn/label.h"
#include "gn/source_file.h"
#include "gn/token.h"
#include "util/msg_loop.h"
#include "util/worker_pool.h"

class Target;

// Maintains the thread pool and error state.
class Scheduler {
 public:
  Scheduler();
  ~Scheduler();

  bool Run();

  MsgLoop* task_runner() {
    DCHECK(main_thread_run_loop_);
    return main_thread_run_loop_;
  }

  InputFileManager* input_file_manager() { return input_file_manager_.get(); }

  bool verbose_logging() const { return verbose_logging_; }
  void set_verbose_logging(bool v) { verbose_logging_ = v; }

  // TODO(brettw) data race on this access (benign?).
  bool is_failed() const { return is_failed_; }

  void Log(const std::string& verb, const std::string& msg);
  void FailWithError(const Err& err);

  void ScheduleWork(std::function<void()> work);

  void Shutdown();

  // Declares that the given file was read and affected the build output.
  //
  // Some consumers expect provided path to be absolute.kk
  //
  // TODO(brettw) this is global rather than per-BuildSettings. If we
  // start using >1 build settings, then we probably want this to take a
  // BuildSettings object so we know the dependency on a per-build basis.
  // If moved, most of the Add/Get functions below should move as well.
  void AddGenDependency(const base::FilePath& file);
  std::vector<base::FilePath> GetGenDependencies() const;

  // Tracks calls to write_file for resolving with the unknown generated
  // inputs (see AddUnknownGeneratedInput below).
  void AddWrittenFile(const SourceFile& file);

  // Schedules a file to be written due to a target setting write_runtime_deps.
  void AddWriteRuntimeDepsTarget(const Target* entry);
  std::vector<const Target*> GetWriteRuntimeDepsTargets() const;
  bool IsFileGeneratedByWriteRuntimeDeps(const OutputFile& file) const;

  // Tracks generated_file calls.
  void AddGeneratedFile(const Target* target, const OutputFile& file);
  bool IsFileGeneratedByTarget(const OutputFile& file) const;

  // Returns the collection of generated files. The associated target is
  // tracked as well in order to determine whether the generated file actually
  // was created (i.e., whether the target was included in the build). This
  // target must only be consulted after the graph is complete.
  std::multimap<OutputFile, const Target*> GetGeneratedFiles() const;

  // Unknown generated inputs are files that a target declares as an input
  // in the output directory, but which aren't generated by any dependency.
  //
  // Some of these files will be files written by write_file and will be
  // GenDependencies (see AddWrittenFile above). There are OK and include
  // things like response files for scripts. Others cases will be ones where
  // the file is generated by a target that's not a dependency.
  //
  // In order to distinguish these two cases, the checking for these input
  // files needs to be done after all targets are complete. This also has the
  // nice side effect that if a target generates the file we can find it and
  // tell the user which dependency is missing.
  //
  // The result returned by GetUnknownGeneratedInputs will not count any files
  // that were written by write_file during execution.
  void AddUnknownGeneratedInput(const Target* target, const SourceFile& file);
  std::multimap<SourceFile, const Target*> GetUnknownGeneratedInputs() const;
  void ClearUnknownGeneratedInputsAndWrittenFiles();  // For testing.

  // We maintain a count of the things we need to do that works like a
  // refcount. When this reaches 0, the program exits.
  void IncrementWorkCount();
  void DecrementWorkCount();

  void SuppressOutputForTesting(bool suppress);

 private:
  void LogOnMainThread(const std::string& verb, const std::string& msg);
  void FailWithErrorOnMainThread(const Err& err);

  void DoTargetFileWrite(const Target* target);

  void OnComplete();

  // Waits for tasks scheduled via ScheduleWork() to complete their execution.
  void WaitForPoolTasks();

  MsgLoop* main_thread_run_loop_;

  scoped_refptr<InputFileManager> input_file_manager_;

  bool verbose_logging_ = false;

  base::AtomicRefCount work_count_;

  // Number of tasks scheduled by ScheduleWork() that haven't completed their
  // execution.
  base::AtomicRefCount pool_work_count_;

  // Lock for |pool_work_count_cv_|.
  std::mutex pool_work_count_lock_;

  // Condition variable signaled when |pool_work_count_| reaches zero.
  std::condition_variable pool_work_count_cv_;

  WorkerPool worker_pool_;

  mutable std::mutex lock_;
  bool is_failed_ = false;

  bool suppress_output_for_testing_ = false;

  // Used to track whether the worker pool has been shutdown. This is necessary
  // to clean up after tests that make a scheduler but don't run the message
  // loop.
  bool has_been_shutdown_ = false;

  // Protected by the lock. See the corresponding Add/Get functions above.
  std::vector<base::FilePath> gen_dependencies_;
  std::vector<SourceFile> written_files_;
  std::vector<const Target*> write_runtime_deps_targets_;
  std::multimap<SourceFile, const Target*> unknown_generated_inputs_;
  std::multimap<OutputFile, const Target*> generated_files_;

  Scheduler(const Scheduler&) = delete;
  Scheduler& operator=(const Scheduler&) = delete;
};

extern Scheduler* g_scheduler;

#endif  // TOOLS_GN_SCHEDULER_H_
