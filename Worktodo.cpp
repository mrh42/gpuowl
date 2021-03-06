// GpuOwL, a Mersenne primality tester. Copyright (C) Mihai Preda.

#include "Worktodo.h"

#include "Task.h"
#include "File.h"
#include "common.h"
#include "Args.h"

#include <cassert>
#include <cstring>
#include <string>
#include <optional>

namespace {

std::optional<Task> parse(const std::string& line) {
  u32 exp = 0;

  u32 bitLo = 0;
  int pos = 0;
  u32 wantsPm1 = 0;
  u64 B1 = 0, B2 = 0;
  
  const char* tail = line.c_str();
  if (sscanf(tail, "B1=%ul,B2=%ul;%n", &B1, &B2, &pos) == 2 ||
      sscanf(tail, "B1=%ul;%n", &B1, &pos) == 1) {
    tail += pos;
  }

  char kindStr[32] = {0};
  if(sscanf(tail, "%7[a-zA-Z]=%n", kindStr, &pos) == 1) {
    string kind = kindStr;
    tail += pos;
    if (kind == "PRP" || kind == "PFactor" || kind == "Pfactor") {
      char AIDStr[64] = {0};
      if (sscanf(tail, "%32[0-9a-fA-FN/],1,2,%u,-1,%u,%u", AIDStr, &exp, &bitLo, &wantsPm1) == 4
          || (AIDStr[0]=0, sscanf(tail, "%u", &exp)) == 1) {
        string AID = AIDStr;
        if (AID == "N/A" || AID == "0") { AID = ""; }
        return {{kind == "PRP" ? Task::PRP : Task::PM1, exp, AID, line, B1, B2, bitLo, wantsPm1}};
      }
    }
  }
  log("worktodo.txt line ignored: \"%s\"\n", rstripNewline(line).c_str());
  return std::nullopt;
}

void remove(const std::string& s) { ::remove(s.c_str()); }
void rename(const std::string& a, const std::string& b) { ::rename(a.c_str(), b.c_str()); }

bool deleteLine(const std::string& fileName, const std::string& targetLine) {
  assert(!targetLine.empty());
  bool lineDeleted = false;
  {
    auto fo{File::openWrite(fileName + "-tmp")};
    for (const string& line : File::openRead(fileName, true)) {
      if (!lineDeleted && line == targetLine) {
        lineDeleted = true;
      } else {
        fo.write(line);
      }
    }
  }

  if (!lineDeleted) {
    log("'%s': could not find the line '%s' to delete\n", fileName.c_str(), targetLine.c_str());
    return false;
  }
  remove(fileName + "-bak");
  rename(fileName, fileName + "-bak");
  rename(fileName + "-tmp", fileName);  
  return true;
}

std::optional<Task> firstGoodTask(const std::string& fileName) {
  for (const string& line : File::openRead(fileName)) {
    if (optional<Task> maybeTask = parse(line)) { return maybeTask; }
  }
  return nullopt;
}

}

std::optional<Task> Worktodo::getTask(Args &args) {
  string worktodoTxt = "worktodo.txt";
  
 again:
  // Try to get a task from the local worktodo.txt
  if (optional<Task> task = firstGoodTask(worktodoTxt)) {
    if (task->kind == Task::PRP && task->wantsPm1) {
      // Some worktodo tasks can be expanded into subtasks:
      // PRP with wantsPm1>0 is expanded into a sequence of P-1 followed by PRP with wantsPm1==0.
      Task pm1{Task::PM1, task->exponent, task->AID, "", task->B1, task->B2, task->bitLo, task->wantsPm1};
      pm1.adjustBounds(args);
      task->wantsPm1 = 0;
      // File::append(worktodoTxt, "#"s + task.line);
      File::append(worktodoTxt, string(pm1) + '\n');
      File::append(worktodoTxt, string(*task) + '\n');
      deleteLine(worktodoTxt, task->line);
      goto again;
    }
    
    task->adjustBounds(args);
    return task;
  }
  
  if (!args.masterDir.empty()) {
    string globalWorktodo = args.masterDir + '/' + worktodoTxt;
    if (optional<Task> task = firstGoodTask(globalWorktodo)) {
      File::append(worktodoTxt, task->line);
      deleteLine(globalWorktodo, task->line);
      goto again;
    }
  }
  
  return std::nullopt;
}

void Worktodo::deletePRP(u32 exponent) {
  std::string fileName = "worktodo.txt";
  bool changed = false;
  {
    auto fo{File::openWrite(fileName + "-tmp")};
    for (const string& line : File::openRead(fileName, true)) {
      if (optional<Task> task = parse(line); task && task->exponent == exponent && task->kind == Task::PRP) {
        changed = true;
        log("task removed: \"%s\"\n", rstripNewline(line).c_str());
      } else {
        fo.write(line);
      }
    }
  }

  if (changed) {
    remove(fileName + "-bak");
    rename(fileName, fileName + "-bak");
    rename(fileName + "-tmp", fileName);  
  }
}

bool Worktodo::deleteTask(const Task &task) {
  // Some tasks don't originate in worktodo.txt and thus don't need deleting.
  if (task.line.empty()) { return true; }
  return deleteLine("worktodo.txt", task.line);
}
