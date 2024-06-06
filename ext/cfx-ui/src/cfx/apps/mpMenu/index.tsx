import './bootstrap';
import { HashRouter, Route, Routes } from 'react-router-dom';

import { ChangelogPage } from 'cfx/apps/mpMenu/pages/ChangelogPage/ChangelogPage';
import { HomePage } from 'cfx/apps/mpMenu/pages/HomePage/HomePage';
import { registerMpMenuServersService } from 'cfx/apps/mpMenu/services/servers/servers.mpMenu';
import { startBrowserApp } from 'cfx/base/createApp';
import { IActivityService, registerActivityService } from 'cfx/common/services/activity/activity.service';
import { registerAnalyticsService } from 'cfx/common/services/analytics/analytics.service';
import { GTMAnalyticsProvider } from 'cfx/common/services/analytics/providers/gtm';
import { IIntlService } from 'cfx/common/services/intl/intl.service';
import { registerLogService } from 'cfx/common/services/log/logService';
import { ConsoleLogProvider } from 'cfx/common/services/log/providers/consoleLogProvider';
import { ServersListType } from 'cfx/common/services/servers/lists/types';
import { IServersService } from 'cfx/common/services/servers/servers.service';
import { IServersBoostService } from 'cfx/common/services/servers/serversBoost.service';
import { IServersConnectService } from 'cfx/common/services/servers/serversConnect.service';
import { IServersStorageService } from 'cfx/common/services/servers/serversStorage.service';
import { registerSettingsService } from 'cfx/common/services/settings/settings.common';
import { ISettingsService, ISettingsUIService } from 'cfx/common/services/settings/settings.service';
import { IUiService } from 'cfx/common/services/ui/ui.service';
import { timeout } from 'cfx/utils/async';

import { MpMenuApp } from './components/MpMenuApp/MpMenuApp';
import { mpMenu } from './mpMenu';
import { Handle404 } from './pages/404';
import { MpMenuServerDetailsPage } from './pages/ServerDetailsPage/ServerDetailsPage';
import { MpMenuServersPage } from './pages/ServersPage/ServersPage';
import { IAuthService, registerAuthService } from './services/auth/auth.service';
import { registerChangelogService } from './services/changelog/changelog.service';
import { registerConvarService } from './services/convars/convars.service';
import { registerDiscourseService } from './services/discourse/discourse.service';
import { registerMpMenuIntlService } from './services/intl/intl.mpMenu';
import { registerLegalService } from './services/legal/legal.service';
import { registerLinkedIdentitiesService } from './services/linkedIdentities/linkedIdentities.service';
import {
  IPlatformStatusService,
  registerPlatformStatusService,
} from './services/platformStatus/platformStatus.service';
import { registerSentryService } from './services/sentry/sentry.service';
import { SentryLogProvider } from './services/sentry/sentryLogProvider';
import { registerHomeScreenServerList } from './services/servers/list/HomeScreenServerList.service';
import {
  MpMenuLocalhostServerService,
  registerMpMenuLocalhostServerService,
} from './services/servers/localhostServer.mpMenu';
import { registerMpMenuServersBoostService } from './services/servers/serversBoost.mpMenu';
import { registerMpMenuServersConnectService } from './services/servers/serversConnect.mpMenu';
import { registerMpMenuServersReviewsService } from './services/servers/serversReviews.mpMenu';
import { registerMpMenuServersStorageService } from './services/servers/serversStorage.mpMenu';
import { registerMpMenuUiService } from './services/ui/ui.mpMenu';
import { IUiMessageService, registerUiMessageService } from './services/uiMessage/uiMessage.service';
import { DEFAULT_GAME_SETTINGS_CATEGORY_ID, GAME_SETTINGS } from './settings';
import { shutdownLoadingSplash } from './utils/loadingSplash';

startBrowserApp({
  defineServices(container) {
    registerSentryService(container);

    registerLogService(container, [ConsoleLogProvider, SentryLogProvider]);

    registerAnalyticsService(container, [GTMAnalyticsProvider]);

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
      listTypes: [ServersListType.All, ServersListType.Supporters, ServersListType.History, ServersListType.Favorites],
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
