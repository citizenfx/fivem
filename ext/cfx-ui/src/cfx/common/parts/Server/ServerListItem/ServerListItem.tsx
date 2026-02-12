import {
  Button,
  CountryFlag,
  Flex,
  Icons,
  Indicator,
  Interactive,
  Loaf,
  Title,
  clsx,
} from '@cfx-dev/ui-components';
import formatDistance from 'date-fns/formatDistance';
import { observer } from 'mobx-react-lite';
import React from 'react';
import { useNavigate } from 'react-router-dom';

import { playSfx, Sfx } from 'cfx/apps/mpMenu/utils/sfx';
import { useService, useServiceOptional } from 'cfx/base/servicesContainer';
import { useEventHandler } from 'cfx/common/services/analytics/analytics.service';
import { EventActionNames, ElementPlacements, isFeaturedElementPlacement } from 'cfx/common/services/analytics/types';
import { $L } from 'cfx/common/services/intl/l10n';
import { getServerDetailsLink, isServerLiveLoading } from 'cfx/common/services/servers/helpers';
import { IServersService } from 'cfx/common/services/servers/servers.service';
import { IServersBoostService } from 'cfx/common/services/servers/serversBoost.service';
import { IServerView } from 'cfx/common/services/servers/types';
import { stopPropagation } from 'cfx/utils/domEvents';
import { useServerCountryTitle } from 'cfx/utils/hooks';

import { ServerBoostButton } from '../ServerBoostButton/ServerBoostButton';
import { ServerIcon } from '../ServerIcon/ServerIcon';
import { ServerPlayersCount } from '../ServerPlayersCount/ServerPlayersCount';
import { ServerPower } from '../ServerPower/ServerPower';
import { ServerPowerTotalButton } from '../ServerPowerTotalButton/ServerPowerTotalButton';
import { ServerTitle } from '../ServerTitle/ServerTitle';

import s from './ServerListItem.module.scss';

export interface ServerListItemProps {
  server: IServerView | undefined;

  pinned?: boolean;
  standalone?: boolean;

  hideTags?: boolean;
  hideActions?: boolean;
  hideCountryFlag?: boolean;

  elementPlacement?: ElementPlacements;
}

export const ServerListItem = observer(function ServerListItem(props: ServerListItemProps) {
  const {
    server,
    pinned = false,
    standalone = false,
    hideTags = false,
    hideActions = false,
    hideCountryFlag = false,
    elementPlacement = ElementPlacements.Unknown,
  } = props;

  const navigate = useNavigate();
  const eventHandler = useEventHandler();

  const ServersBoostService = useServiceOptional(IServersBoostService);

  const isBoostedByUser = ServersBoostService?.currentBoost?.address === server?.id;

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

  const showCountryFlag = !hideCountryFlag;

  const showTags = !hideTags && !!server.tags;

  const rootClassName = clsx(s.root, {
    [s.standalone]: standalone,
    [s.boosted]: boostPower,
    [s.pinned]: pinned,
    [s.platinum]: server.premium === 'pt',
  });

  return (
    <Interactive onClick={handleClick} className={rootClassName}>
      <ServerIcon type="list" server={server} loading={isLoading} className={s.icon} />

      <div className={s.title}>
        {isOffline && (
          <Flex centered="axis" gap="small">
            <Loaf size="small" color="error">
              {$L('#Server_Offline')}
            </Loaf>
            <ServerTitle title={server.projectName || server.hostname} />
          </Flex>
        )}
        {!isOffline && (
          <ServerTitle title={server.projectName || server.hostname} />
        )}

        {!!description && (
          <span className={s.description}>{description}</span>
        )}
      </div>

      {showTags && (
        <Tags server={server} />
      )}

      {!hideActions && (
        <div className={clsx(s.actions, s['show-on-hover'])}>
          <ServerPower server={server} />
          <Flex gap="none">
            <ServerBoostButton
              className={clsx(s.serverboostbutton, { [s['serverboostbutton-active']]: isBoostedByUser })}
              server={server}
            />
            {isBoostedByUser && (
              <ServerPowerTotalButton className={s.serverpowerbutton} server={server} />
            )}
          </Flex>
        </div>
      )}

      <LastConnectedAt id={server.id} />

      {showCountryFlag && (
        <div className={clsx(s.decorator)}>
          <CountryFlag forceShow title={countryTitle} country={server.localeCountry} />
        </div>
      )}

      <Favorite id={server.id} />

      <div className={s.players}>
        <ServerPlayersCount server={server} />
      </div>
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
        {Icons.serversListHistory} {distanceDate}
      </Loaf>
    </Title>
  );
});
