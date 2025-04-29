import { Page } from "cfx/ui/Layout/Page/Page";
import { Flex } from "cfx/ui/Layout/Flex/Flex";
import { Text } from "cfx/ui/Text/Text";
import { observer } from "mobx-react-lite";
import { AcitivityItemMediaViewerProvider } from "../../components/AcitivityItemMediaViewer/AcitivityItemMediaViewer.context";
import { ThemeManager } from "../../parts/ThemeManager/ThemeManager";
import { XacThucOTP } from "./ServerPreview/XacThucOTP";
import { useService, useServiceOptional } from "cfx/base/servicesContainer";
import { timeout } from "cfx/utils/async";
import { Pad } from "cfx/ui/Layout/Pad/Pad";
import { Indicator } from "cfx/ui/Indicator/Indicator";
import { useServerPreviewService } from "cfx/common/services/servers/serverPreview.service";
import { useServersService } from "cfx/common/services/servers/servers.service";
import { LegacyConnectingModal } from "../../parts/LegacyConnectingModal/LegacyConnectingModal";
import { LegacyUiMessageModal } from "../../parts/LegacyUiMessageModal/LegacyUiMessageModal";

export const VNGtaCodePage = observer(function VNGtaCodePage() {
    const ServerPreviewService = useServerPreviewService();
    
    const isLoading = !ServerPreviewService.currentLoadComplete;

    return (
        <AcitivityItemMediaViewerProvider>
            <ThemeManager />
            <LegacyConnectingModal />
            <LegacyUiMessageModal />
            <Page>
                <Flex fullWidth fullHeight centered gap="xlarge">
                    {isLoading ? loader() : <XacThucOTP/>}
                </Flex>
            </Page>
        </AcitivityItemMediaViewerProvider>
    );

    function loader() {
        return (
            <>
                <Pad>
                <Flex>
                    <Indicator />
                    <Text>
                        Đang tải dữ liệu....
                    </Text>
                </Flex>
                </Pad>
            </>
        );
    }
});