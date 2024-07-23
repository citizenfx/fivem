import {
  Button,
  CountryFlag,
  Icons,
  Indicator,
  Interactive,
  Loaf,
  PremiumBadge,
  Title,
  clsx,
} from '@cfx-dev/ui-components';
import formatDistance from 'date-fns/formatDistance';
import { observer } from 'mobx-react-lite';
import React from 'react';
import { useNavigate } from 'react-router-dom';

import { playSfx, Sfx } from 'cfx/apps/mpMenu/utils/sfx';
import { useService } from 'cfx/base/servicesContainer';
import { useEventHandler } from 'cfx/common/services/analytics/analytics.service';
import { EventActionNames, ElementPlacements, isFeaturedElementPlacement } from 'cfx/common/services/analytics/types';
import { $L } from 'cfx/common/services/intl/l10n';
import { getServerDetailsLink, isServerLiveLoading, showServerPremiumBadge } from 'cfx/common/services/servers/helpers';
import { IServersService } from 'cfx/common/services/servers/servers.service';
import { IServerView } from 'cfx/common/services/servers/types';
import { stopPropagation } from 'cfx/utils/domEvents';
import { useServerCountryTitle } from 'cfx/utils/hooks';

import { ServerBoostButton } from '../ServerBoostButton/ServerBoostButton';
import { ServerIcon } from '../ServerIcon/ServerIcon';
import { ServerPlayersCount } from '../ServerPlayersCount/ServerPlayersCount';
import { ServerPower } from '../ServerPower/ServerPower';
import { ServerTitle } from '../ServerTitle/ServerTitle';

import s from './ServerListItem.module.scss';

export interface ServerListItemProps {
  server: IServerView | undefined;

  pinned?: boolean;
  standalone?: boolean;

  hideTags?: boolean;
  hideActions?: boolean;
  hideCountryFlag?: boolean;
  hidePremiumBadge?: boolean;

  elementPlacement?: ElementPlacements;
  descriptionUnderName?: boolean;
}

export const ServerListItem = observer(function ServerListItem(props: ServerListItemProps) {
  const {
    server,
    pinned = false,
    standalone = false,
    hideTags = false,
    hideActions = false,
    hideCountryFlag = false,
    hidePremiumBadge = false,
    descriptionUnderName = false,
    elementPlacement = ElementPlacements.Unknown,
  } = props;

  const navigate = useNavigate();
  const eventHandler = useEventHandler();

  const handleClick = React.useCallback(() => {
    if (!server) {
      return;
    }

    const serverLink = getServerDetailsLink(server);

    eventHandler({
      action: EventActionNames.ServerSelect,
      properties: {
        element_placement: elementPlacement,
        server_id: server.id,
        server_name: server.projectName || server.hostname,
        server_type: isFeaturedElementPlacement(elementPlacement)
          ? 'featured'
          : undefined,
        text: 'Server list Item',
        link_url: serverLink,
      },
    });

    navigate(serverLink);
  }, [navigate, server, eventHandler, elementPlacement]);

  const countryTitle = useServerCountryTitle(server?.locale, server?.localeCountry);

  if (!server) {
    return (
      <div className={s.root}>
        <Indicator />
      </div>
    );
  }

  const description = server.projectDescription;

  const burstPower = server.burstPower || 0;
  const boostPower = (server.upvotePower || 0) - burstPower;

  const isOffline = Boolean(server.offline);
  const isLoading = isServerLiveLoading(server);

  const showPremiumBadge = !hidePremiumBadge && showServerPremiumBadge(server.premium);
  const showCountryFlag = !hideCountryFlag;
  const showDecorator = showPremiumBadge || showCountryFlag;

  const showTags = !hideTags && !!server.tags;

  const rootClassName = clsx(s.root, {
    [s.standalone]: standalone,
    [s.boosted]: boostPower,
    [s.pinned]: pinned,
    [s.platinum]: server.premium === 'pt',
    [s['description-under-name']]: descriptionUnderName,
  });

  return (
    <Interactive onClick={handleClick} className={rootClassName}>
      <ServerIcon type="list" server={server} loading={isLoading} className={s.icon} />

      {isOffline && (
        <Loaf size="small" color="error">
          {$L('#Server_Offline')}
        </Loaf>
      )}

      {pinned && (
        <Title title={$L('#Server_FeaturedServer_Title')}>
          <div className={s.pin}>{Icons.serversFeatured}</div>
        </Title>
      )}

      <div className={s.title}>
        <ServerTitle title={server.projectName || server.hostname} />

        {!!description && (
          <>
            {descriptionUnderName && (
              <br />
            )}
            <span className={s.description}>{description}</span>
          </>
        )}
      </div>

      {showTags && (
        <Tags server={server} />
      )}

      {!hideActions && (
        <div className={clsx(s.actions, s['show-on-hover'])}>
          <ServerPower server={server} />

          <ServerBoostButton server={server} />
        </div>
      )}

      <LastConnectedAt id={server.id} />

      {showDecorator && (
        <div className={clsx(s.decorator)}>
          {showPremiumBadge && (
            <PremiumBadge level={server.premium as any} />
          )}

          <CountryFlag forceShow title={countryTitle} country={server.localeCountry} />
        </div>
      )}

      <Favorite id={server.id} />

      <div className={s.players}>
        <ServerPlayersCount server={server} />
      </div>

      {/* <Density server={server} /> */}

      {/* SPACER */}
      <div />
    </Interactive>
  );
});

const Tags = observer(function Tags({
  server,
}: { server: IServerView }) {
  const ServersService = useService(IServersService);

  return (
    <div className={clsx(s.tags, s['hide-on-hover'])}>
      {ServersService.getTagsForServer(server).map((tag, i) => (
        // eslint-disable-next-line react/no-array-index-key
        <Loaf key={tag + i} size="small">
          {tag}
        </Loaf>
      ))}
    </div>
  );
});

// eslint-disable-next-line @typescript-eslint/no-unused-vars
function Density({
  server,
}: { server: IServerView }) {
  const density = 0 + Math.round(((server.playersCurrent || 0) / (server.playersMax || 1)) * 100);

  return (
    <div className={s.density} style={{ '--density': `${density}%` } as any}>
      {Math.ceil(((server.playersCurrent || 0) / (server.playersMax || 1)) * 100)}
      &nbsp;
      <span className={s.dim}>%</span>
    </div>
  );
}

const Favorite = observer(function Favorite({
  id,
}: { id: string }) {
  const ServersService = useService(IServersService);
  const favoriteServersList = ServersService.getFavoriteList();

  if (!favoriteServersList) {
    return null;
  }

  const isInFavoriteServersList = favoriteServersList.isIn(id);

  const handleClick = () => {
    if (__CFXUI_USE_SOUNDS__) {
      if (isInFavoriteServersList) {
        playSfx(Sfx.Click4);
      } else {
        playSfx(Sfx.Click2);
      }
    }

    favoriteServersList.toggleIn(id);
  };

  const rootClassName = clsx({
    [s['fade-in-on-hover']]: !isInFavoriteServersList,
  });

  return (
    <div className={rootClassName}>
      <Button
        size="small"
        icon={isInFavoriteServersList
          ? Icons.favoriteActive
          : Icons.favoriteInactive}
        onClick={stopPropagation(handleClick)}
      />
    </div>
  );
});

const LastConnectedAt = observer(function LastConnectedAt({
  id,
}: { id: string }) {
  const ServersService = useService(IServersService);
  const historyList = ServersService.getHistoryList();

  if (!historyList) {
    return null;
  }

  const lastConnectedAt = historyList.getLastConnectedAt(id);

  if (!lastConnectedAt) {
    return null;
  }

  const fullDate = lastConnectedAt.toUTCString();
  const distanceDate = formatDistance(lastConnectedAt, new Date(), { addSuffix: true });

  return (
    <Title fixedOn="bottom" title={`Last connected at ${fullDate}`}>
      <Loaf bright size="small" className={s['hide-on-hover']}>
        {$L('#Server_LastPlayed')}: {distanceDate}
      </Loaf>
    </Title>
  );
});
