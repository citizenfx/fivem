import * as React from 'react';
import { statusesApi } from 'shared/events';
import { sendApiMessage } from 'utils/api';
import { useApiMessage } from 'utils/hooks';

export type StatusContext = Record<string, any>;

export const StatusContext = React.createContext<StatusContext>({});

export const StatusContextProvider = React.memo(function StatusContextProvider({ children }) {
  const [statuses, setStatuses] = React.useState<StatusContext>({});

  React.useEffect(() => sendApiMessage(statusesApi.ack), []);

  useApiMessage(statusesApi.statuses, (newStatuses) => {
    setStatuses(newStatuses);
  }, [setStatuses]);

  useApiMessage(statusesApi.update, ([statusName, statusContent]) => {
    setStatuses({
      ...statuses,
      [statusName]: statusContent,
    });
  }, [statuses, setStatuses]);

  return (
    <StatusContext.Provider value={statuses}>
      {children}
    </StatusContext.Provider>
  );
});

export const useStatus = <T extends any>(statusName: string, defaultContent: T): T => {
  const statuses = React.useContext(StatusContext);

  const statusContent = statuses[statusName];

  return typeof statusContent === 'undefined'
    ? defaultContent
    : statusContent;
};
