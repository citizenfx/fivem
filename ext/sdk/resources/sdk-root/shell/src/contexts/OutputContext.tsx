import * as React from 'react';
import { outputApi } from 'shared/api.events';
import { useApiMessage } from 'utils/hooks';

export interface OutputContext {
  outputs: Record<string, string>;
  outputsLabels: Record<string, string>;
}
export const OutputContext = React.createContext<OutputContext>({
  outputs: {},
  outputsLabels: {},
});

export const OutputContextProvider = React.memo(function OutputContextProvider({ children }) {
  const [outputs, setOutputs] = React.useState<Record<string, string>>({});
  const [outputsLabels, setOutputsLabels] = React.useState<Record<string, string>>({});

  useApiMessage(outputApi.output, ({ channelId, data }) => {
    const newOutputs = { ...outputs };

    newOutputs[channelId] = (newOutputs[channelId] || '') + data;

    setOutputs(newOutputs);
  }, [outputs, setOutputs]);

  useApiMessage(outputApi.outputLabel, ({ channelId, label }) => {
    const newOutputsLabels = { ...outputsLabels };

    newOutputsLabels[channelId] = label;

    setOutputsLabels(newOutputsLabels);
  }, [outputsLabels, setOutputsLabels]);

  useApiMessage(outputApi.flush, ({ channelId }) => {
    const newOutputs = { ...outputs };
    const newOutputsLabels = { ...outputsLabels };

    delete newOutputs[channelId];
    delete newOutputsLabels[channelId];

    setOutputs(newOutputs);
    setOutputsLabels(newOutputsLabels);
  }, [outputs, outputsLabels, setOutputs, setOutputsLabels]);

  const value: OutputContext = {
    outputs,
    outputsLabels,
  };

  return (
    <OutputContext.Provider value={value}>
      {children}
    </OutputContext.Provider>
  );
});
