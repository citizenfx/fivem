import * as React from 'react';
import { taskReporterApi } from 'shared/api.events';
import { TaskData, TaskId } from 'shared/task.types';
import { sendApiMessage } from 'utils/api';
import { useApiMessage } from 'utils/hooks';
import { logger } from 'utils/logger';

const log = logger('TaskContext');

export type TasksState = TaskData[];

export interface TaskContext {
  tasks: TasksState,
  taskIndices: Record<TaskId, number>,
}
export const TaskContext = React.createContext<TaskContext>({
  tasks: [],
  taskIndices: {},
});

export const TaskContextProvider = React.memo(function TaskContextProvider({ children }) {
  const [tasks, setTasks] = React.useState<TasksState>([]);

  const taskIndicesRef = React.useRef({});
  const tasksRef = React.useRef(tasks);

  tasksRef.current = tasks;
  taskIndicesRef.current = React.useMemo(() => tasks.reduce((acc, task, index) => {
    acc[task.id] = index;
    return acc;
  }, taskIndicesRef.current), [tasks]);

  React.useEffect(() => sendApiMessage(taskReporterApi.ackTasks), []);

  useApiMessage(taskReporterApi.tasks, (tasks: TasksState) => {
    taskIndicesRef.current = tasks.reduce((acc, task, index) => {
      acc[task.id] = index;
      return acc;
    }, taskIndicesRef.current);
    tasksRef.current = tasks;
    setTasks(tasks);
  }, [setTasks]);
  useApiMessage(taskReporterApi.taskAdded, (task: TaskData) => {
    if (taskIndicesRef.current[task.id] > -1) {
      return log('Attempt to add task that already exist', task, tasks);
    }

    taskIndicesRef.current[task.id] = tasksRef.current.length;

    const newTasks = [...tasksRef.current, task];

    tasksRef.current = newTasks;
    setTasks(newTasks);
  }, [setTasks]);
  useApiMessage(taskReporterApi.taskDeleted, (taskId: TaskId) => {
    const index = taskIndicesRef.current[taskId];
    if (index > -1) {
      delete taskIndicesRef.current[taskId];
      const newTasks = [...tasksRef.current];
      newTasks.splice(index, 1);

      tasksRef.current = newTasks;
      setTasks(newTasks);
    } else {
      log('Can not delete task as its index is unknown', taskId);
    }
  }, [setTasks]);
  useApiMessage(taskReporterApi.taskChanged, (task: TaskData) => {
    const index = taskIndicesRef.current[task.id];
    if (index > -1) {
      const newTasks = [...tasksRef.current];

      newTasks[index] = task;

      tasksRef.current = newTasks;
      setTasks(newTasks);
    } else {
      log('Can not change task as its index is unknown', task.id);
    }
  }, [setTasks]);

  const value = {
    tasks,
    taskIndices: taskIndicesRef.current,
  };

  return (
    <TaskContext.Provider value={value}>
      {children}
    </TaskContext.Provider>
  );
});

export const useTask = (taskId: TaskId): TaskData | null => {
  const { tasks, taskIndices } = React.useContext(TaskContext);

  return tasks[taskIndices[taskId]] || null;
};
