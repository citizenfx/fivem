import * as React from 'react';
import { useSdkMessageEmitter } from 'utils/hooks';

export const SdkMessageManager = React.memo(function SdkMessageManager() {
  const handleSdkMessage = useSdkMessageEmitter();

  React.useEffect(() => {
    const handler = (event) => {
      if (typeof event.data !== 'object' || event.data === null) {
        return;
      }

      const { type, data } = event.data;
      if (!type) {
        return;
      }

      handleSdkMessage(type, data);
    };

    window.addEventListener('message', handler);

    return () => window.removeEventListener('message', handler);
  }, [handleSdkMessage]);

  return null;
});
