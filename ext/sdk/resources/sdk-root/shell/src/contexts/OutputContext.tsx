import * as React from 'react';
import { outputApi } from 'shared/api.events';
import { useApiMessage } from 'utils/hooks';

export interface OutputContext {
  outputs: Record<string, string>;
}
export const OutputContext = React.createContext<OutputContext>({
  outputs: {},
});

export const OutputContextProvider = React.memo(function OutputContextProvider({ children }) {
  const [outputs, setOutputs] = React.useState<Record<string, string>>({});

  useApiMessage(outputApi.output, ({ channelId, data }) => {
    const newOutputs = { ...outputs };

    newOutputs[channelId] = (newOutputs[channelId] || '') + data;

    setOutputs(newOutputs);
  }, [outputs, setOutputs]);

  useApiMessage(outputApi.flush, ({ channelId }) => {
    const newOutputs = { ...outputs };

    delete newOutputs[channelId];

    setOutputs(newOutputs);
  }, [outputs, setOutputs]);

  const value: OutputContext = {
    outputs,
  };

  return (
    <OutputContext.Provider value={value}>
      {children}
    </OutputContext.Provider>
  );
});
