import { Outlet } from 'react-router-dom';
import { NavBar } from 'cfx/apps/mpMenu/parts/NavBar/NavBar';
import { AuthFlyout } from 'cfx/apps/mpMenu/parts/AuthFlyout/AuthFlyout';
import { SettingsFlyout } from 'cfx/common/parts/Settings/SettingsFlyout/SettingsFlyout';
import { ThemeManager } from 'cfx/apps/mpMenu/parts/ThemeManager/ThemeManager';
import { LegacyConnectingModal } from 'cfx/apps/mpMenu/parts/LegacyConnectingModal/LegacyConnectingModal';
import { LegacyUiMessageModal } from 'cfx/apps/mpMenu/parts/LegacyUiMessageModal/LegacyUiMessageModal';
import { ServerBoostModal } from 'cfx/apps/mpMenu/parts/ServerBoostModal/ServerBoostModal';
import s from './GameApp.module.scss';

export function GameApp() {
  return (
    <>
      <ThemeManager />

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
    </>
  );
}
