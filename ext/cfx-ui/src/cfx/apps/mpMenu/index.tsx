import './bootstrap';
import { HashRouter, Route, Routes } from 'react-router-dom';
import { HomePage } from 'cfx/apps/mpMenu/pages/HomePage/HomePage';
import { ChangelogPage } from 'cfx/apps/mpMenu/pages/ChangelogPage/ChangelogPage';
import { ServersListType } from 'cfx/common/services/servers/lists/types';
import { MpMenuApp } from './components/MpMenuApp/MpMenuApp';
import { startBrowserApp } from 'cfx/base/createApp';
import { MpMenuServersPage } from './pages/ServersPage/ServersPage';
import { MpMenuServerDetailsPage } from './pages/ServerDetailsPage/ServerDetailsPage';
import { DEFAULT_GAME_SETTINGS_CATEGORY_ID, GAME_SETTINGS } from './settings';
import { ConsoleLogProvider } from 'cfx/common/services/log/providers/consoleLogProvider';
import { registerLogService } from 'cfx/common/services/log/logService';
import { registerMpMenuServersService } from 'cfx/apps/mpMenu/services/servers/servers.mpMenu';
import { registerSettingsService } from 'cfx/common/services/settings/settings.common';
import { registerMpMenuUiService } from './services/ui/ui.mpMenu';
import { registerMpMenuServersStorageService } from './services/servers/serversStorage.mpMenu';
import { registerConvarService } from './services/convars/convars.service';
import { registerDiscourseService } from './services/discourse/discourse.service';
import { registerLinkedIdentitiesService } from './services/linkedIdentities/linkedIdentities.service';
import { registerMpMenuServersBoostService } from './services/servers/serversBoost.mpMenu';
import { IAuthService, registerAuthService } from './services/auth/auth.service';
import { registerMpMenuServersReviewsService } from './services/servers/serversReviews.mpMenu';
import { registerChangelogService } from './services/changelog/changelog.service';
import { registerAnalyticsService } from 'cfx/common/services/analytics/analytics.service';
import { registerMpMenuServersConnectService } from './services/servers/serversConnect.mpMenu';
import { IUiMessageService, registerUiMessageService } from './services/uiMessage/uiMessage.service';
import { registerMpMenuIntlService } from './services/intl/intl.mpMenu';
import { MpMenuLocalhostServerService, registerMpMenuLocalhostServerService } from './services/servers/localhostServer.mpMenu';
import { IPlatformStatusService, registerPlatformStatusService } from './services/platformStatus/platformStatus.service';
import { IActivityService, registerActivityService } from 'cfx/common/services/activity/activity.service';
import { mpMenu } from './mpMenu';
import { IServersService } from 'cfx/common/services/servers/servers.service';
import { IUiService } from 'cfx/common/services/ui/ui.service';
import { IIntlService } from 'cfx/common/services/intl/intl.service';
import { ISettingsService, ISettingsUIService } from 'cfx/common/services/settings/settings.service';
import { IServersBoostService } from 'cfx/common/services/servers/serversBoost.service';
import { IServersStorageService } from 'cfx/common/services/servers/serversStorage.service';
import { IServersConnectService } from 'cfx/common/services/servers/serversConnect.service';
import { Handle404 } from './pages/404';
import { registerHomeScreenServerList } from './services/servers/list/HomeScreenServerList.service';
import { registerSentryService } from './services/sentry/sentry.service';
import { registerLegalService } from "./services/legal/legal.service";
import { SentryLogProvider } from './services/sentry/sentryLogProvider';
import { timeout } from 'cfx/utils/async';
import { shutdownLoadingSplash } from "./utils/loadingSplash";

startBrowserApp({
  defineServices(container) {
    registerSentryService(container);

    registerLogService(container, [
      ConsoleLogProvider,
      SentryLogProvider,
    ]);

    registerAnalyticsService(container, []);

    registerLegalService(container);

    registerSettingsService(container, {
      settings: GAME_SETTINGS,
      defaultSettingsCategoryId: DEFAULT_GAME_SETTINGS_CATEGORY_ID,
    });

    registerActivityService(container);

    registerAuthService(container);
    registerConvarService(container);
    registerChangelogService(container);
    registerDiscourseService(container);
    registerUiMessageService(container);
    registerPlatformStatusService(container);
    registerLinkedIdentitiesService(container);

    registerHomeScreenServerList(container);

    registerMpMenuUiService(container);
    registerMpMenuIntlService(container);

    registerMpMenuServersService(container, {
      listTypes: [
        ServersListType.All,
        ServersListType.Supporters,
        ServersListType.History,
        ServersListType.Favorites,
      ],
    });
    registerMpMenuServersBoostService(container);
    registerMpMenuServersStorageService(container);
    registerMpMenuServersReviewsService(container);
    registerMpMenuServersConnectService(container);
    registerMpMenuLocalhostServerService(container);
  },

  beforeRender(container) {
    // Pre-resolve critical services
    [
      IUiService,
      IAuthService,
      IIntlService,
      IServersService,
      IActivityService,
      ISettingsService,
      IUiMessageService,
      ISettingsUIService,
      IServersBoostService,
      IPlatformStatusService,
      IServersStorageService,
      IServersConnectService,

      MpMenuLocalhostServerService as any,
    ].forEach((serviceId) => container.get(serviceId));
  },

  render: () => (
    <HashRouter>
      <Routes>
        <Route path="" element={<MpMenuApp />}>
          <Route index element={<HomePage />} />

          <Route path="changelog" element={<ChangelogPage />} />

          <Route path="servers">
            <Route index element={<MpMenuServersPage listType={ServersListType.All} />} />
            <Route path="favorites" element={<MpMenuServersPage listType={ServersListType.Favorites} />} />
            <Route path="history" element={<MpMenuServersPage listType={ServersListType.History} />} />
            <Route path="premium" element={<MpMenuServersPage listType={ServersListType.Supporters} />} />

            <Route path="detail/*" element={<MpMenuServerDetailsPage />} />
          </Route>

          <Route path="*" element={<Handle404 />} />
        </Route>
      </Routes>
    </HashRouter>
  ),

  afterRender() {
    mpMenu.invokeNative('backfillEnable');
    mpMenu.invokeNative('loadWarning');

    if (__CFXUI_DEV__) {
      mpMenu.invokeNative('executeCommand', 'nui_devtools mpMenu');
    }

    mpMenu.showGameWindow();

    // Not using await here so app won't wait for this to end
    timeout(1000).then(shutdownLoadingSplash);
  },
});
