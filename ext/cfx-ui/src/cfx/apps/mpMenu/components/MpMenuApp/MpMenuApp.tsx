import { Outlet } from 'react-router-dom';
import { NavBar } from 'cfx/apps/mpMenu/parts/NavBar/NavBar';
import { AuthFlyout } from 'cfx/apps/mpMenu/parts/AuthFlyout/AuthFlyout';
import { SettingsFlyout } from 'cfx/apps/mpMenu/parts/SettingsFlyout/SettingsFlyout';
import { ThemeManager } from 'cfx/apps/mpMenu/parts/ThemeManager/ThemeManager';
import { LegacyConnectingModal } from 'cfx/apps/mpMenu/parts/LegacyConnectingModal/LegacyConnectingModal';
import { LegacyUiMessageModal } from 'cfx/apps/mpMenu/parts/LegacyUiMessageModal/LegacyUiMessageModal';
import { ServerBoostModal } from 'cfx/apps/mpMenu/parts/ServerBoostModal/ServerBoostModal';
import { AcitivityItemMediaViewerProvider } from '../AcitivityItemMediaViewer/AcitivityItemMediaViewer.context';
import { NavigationTracker } from './PageViewTracker';
import s from './MpMenuApp.module.scss';

export function MpMenuApp() {
  return (
    <AcitivityItemMediaViewerProvider>
      <ThemeManager />
      <NavigationTracker />

      <AuthFlyout />
      <SettingsFlyout />

      <LegacyConnectingModal />
      <LegacyUiMessageModal />

      <ServerBoostModal />

      <div className={s.root}>
        <NavBar />

        <div className={s.outlet}>
          <Outlet />
        </div>
      </div>
    </AcitivityItemMediaViewerProvider>
  );
}
