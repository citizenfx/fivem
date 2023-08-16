import { Page } from "cfx/ui/Layout/Page/Page";
import { Flex } from "cfx/ui/Layout/Flex/Flex";
import { observer } from "mobx-react-lite";
import { useActivityService } from "cfx/common/services/activity/activity.service";
import { LoginContainer } from "./LoginContainer/LoginContainer";
import { AcitivityItemMediaViewerProvider } from "../../components/AcitivityItemMediaViewer/AcitivityItemMediaViewer.context";
import s from './LoginContainerPage.module.scss';
import { ThemeManager } from "../../parts/ThemeManager/ThemeManager";
import { LegacyConnectingModal } from "../../parts/LegacyConnectingModal/LegacyConnectingModal";
import { LegacyUiMessageModal } from "../../parts/LegacyUiMessageModal/LegacyUiMessageModal";

export const LoginPage = observer(function LoginPage() {
    // const ActivityService = useActivityService();
  
    return (
      <AcitivityItemMediaViewerProvider>
        <ThemeManager />
        <LegacyConnectingModal />
        <LegacyUiMessageModal />

        <Page>
            <Flex fullWidth fullHeight vertical centered gap="xlarge">
              <LoginContainer />
            </Flex>
        </Page >
      </AcitivityItemMediaViewerProvider>
    );
  });
  