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
import { mpMenu } from "cfx/apps/mpMenu/mpMenu";

import NTDc from './LoginContainer/NotifyDiscord/NotifyDiscord.module.scss';

import React from "react";

import { ToastContainer, toast } from 'react-toastify';
import 'react-toastify/dist/ReactToastify.css';

export const LoginPage = observer(function LoginPage() {
  console.log(mpMenu.discordIdZ.value, ' xin chao 23')
  if (mpMenu.discordIdZ.value === 0) {
        toast.info(() => (
            <div>
              Vui lòng chờ trong khoản 30 giây để tự cập nhật !
              <br />
              Đang xử lý...
            </div>
          ), {
            autoClose: 30000,
          });
      return (
        <AcitivityItemMediaViewerProvider>
            <ThemeManager />
            <LegacyConnectingModal />
            <LegacyUiMessageModal />

            <Page>
                <Flex fullWidth fullHeight vertical centered gap="xlarge">
                    <span className={NTDc.title_dcnt}>Thông báo</span>
                    <p className={NTDc.text_sumbz}>Bạn chưa mở phần mềm Discord. Vui lòng mở Discord để tiếp tục.</p>
                    {/* <p className={NTDc.text_sumbz}>Nếu bạn đã bật Discord rồi. Hãy thử tắt Discord + Launcher.</p>
                    <p className={NTDc.text_sumbz}>Mở Discord lên trước đợi load  {'->'} Sau đó mở Launcher</p> */}
                    <p className={NTDc.text_sumbz2}>HOẶC bạn đã mở Discord rồi.</p>
                    <p className={NTDc.text_sumbz2}>Bạn chỉ việc để im và chờ trong giây lát</p>
                </Flex>
            </Page>
        </AcitivityItemMediaViewerProvider>
      );
  }
  // React.useEffect(() => {
  //   toast.success('NHẬN DẠNG DISCORD THÀNH CÔNG!');
  // }, []);
  return (
      <AcitivityItemMediaViewerProvider>
          <ThemeManager />
          <LegacyConnectingModal />
          <LegacyUiMessageModal />

          <Page>
              <Flex fullWidth fullHeight vertical centered gap="xlarge">
                  <LoginContainer />
              </Flex>
          </Page>
      </AcitivityItemMediaViewerProvider>
  );
});