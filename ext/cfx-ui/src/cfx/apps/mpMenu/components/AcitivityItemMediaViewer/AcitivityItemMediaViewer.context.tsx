import { ActivityItemContextProvider, IActivityItemContext } from "cfx/ui/ActivityItem/ActivityItem.context";
import { AcitivityItemMediaViewer } from "./AcitivityItemMediaViewer";
import { AcitivityItemMediaViewerState } from "./AcitivityItemMediaViewer.state";

export const AcitivityItemMediaViewerProvider = ({ children }) => {
  return (
    <ActivityItemContextProvider value={AcitivityItemMediaViewerState}>
      <AcitivityItemMediaViewer />

      {children}
    </ActivityItemContextProvider>
  );
};
