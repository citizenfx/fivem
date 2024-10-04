import { ActivityItemContextProvider } from 'cfx/ui/ActivityItem/ActivityItem.context';

import { AcitivityItemMediaViewer } from './AcitivityItemMediaViewer';
import { AcitivityItemMediaViewerState } from './AcitivityItemMediaViewer.state';

export function AcitivityItemMediaViewerProvider({
  children,
}: React.PropsWithChildren) {
  return (
    <ActivityItemContextProvider value={AcitivityItemMediaViewerState}>
      <AcitivityItemMediaViewer />

      {children}
    </ActivityItemContextProvider>
  );
}
