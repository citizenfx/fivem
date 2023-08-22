import { Page } from "cfx/ui/Layout/Page/Page";
import { Flex } from "cfx/ui/Layout/Flex/Flex";
import { observer } from "mobx-react-lite";
import { AcitivityItemMediaViewerProvider } from "../../components/AcitivityItemMediaViewer/AcitivityItemMediaViewer.context";
import { ThemeManager } from "../../parts/ThemeManager/ThemeManager";
import { ServerPreview } from "./ServerPreview/ServerPreview";
import { useService, useServiceOptional } from "cfx/base/servicesContainer";
import { useServerPreviewService } from "../../services/serverPreview/serverPreview.service";

export const DevotedPage = observer(function DevotedPage() {
    const ServerPreviewService = useServerPreviewService();
    ServerPreviewService.getPreviewBackgrounds();
    
    
    
    return (
        <AcitivityItemMediaViewerProvider>
            <ThemeManager />
            <Page>
                <Flex fullWidth fullHeight centered gap="xlarge">
                    <ServerPreview></ServerPreview>
                </Flex>
            </Page>
        </AcitivityItemMediaViewerProvider>
    );
});