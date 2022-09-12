import React from "react";
import formatDistance from 'date-fns/formatDistance';
import { IServerView, ServerViewDetailsLevel } from "cfx/common/services/servers/types";
import { observer } from "mobx-react-lite";
import { Indicator } from "cfx/ui/Indicator/Indicator";
import { clsx } from "cfx/utils/clsx";
import { Button } from "cfx/ui/Button/Button";
import { BsClock } from "react-icons/bs";
import { Title } from "cfx/ui/Title/Title";
import { CountryFlag } from "cfx/ui/CountryFlag/CountryFlag";
import { PremiumBadge } from "cfx/ui/PremiumBadge/PremiumBadge";
import { Loaf } from "cfx/ui/Loaf/Loaf";
import { stopPropagation } from "cfx/utils/domEvents";
import { useNavigate } from "react-router-dom";
import { ServerTitle } from "../ServerTitle/ServerTitle";
import { Icons } from "cfx/ui/Icons";
import { useService, useServiceOptional } from "cfx/base/servicesContainer";
import { IServersService } from "cfx/common/services/servers/servers.service";
import { IServersBoostService } from "cfx/common/services/servers/serversBoost.service";
import { ServerIcon } from "../ServerIcon/ServerIcon";
import { noop } from "cfx/utils/functional";
import { playSfx, Sfx } from "cfx/apps/mpMenu/utils/sfx";
import { ServerPlayersCount } from "../ServerPlayersCount/ServerPlayersCount";
import { showServerPremiumBadge } from "./utils";
import { ServerPower } from "../ServerPower/ServerPower";
import s from './ServerListItem.module.scss';
import { ServerBoostButton } from "../ServerBoostButton/ServerBoostButton";

export interface ServerListItemProps {
  server: IServerView | undefined,

  pinned?: boolean,
  standalone?: boolean,

  hideTags?: boolean,
  hideActions?: boolean,
  hideCountryFlag?: boolean,
  hidePremiumBadge?: boolean,

  descriptionUnderName?: boolean,
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
  } = props;

  const ServersBoostService = useServiceOptional(IServersBoostService);

  const navigate = useNavigate();
  const handleClick = React.useCallback(() => {
    if (!server) {
      return;
    }

    navigate(`/servers/detail/${server.id}`);
  }, [navigate, server]);

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
  const isLoading = server.detailsLevel < ServerViewDetailsLevel.DynamicDataJson && !isOffline;

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
    <div
      onClick={handleClick}
      className={rootClassName}
    >
      <ServerIcon
        type="list"
        server={server.id}
        loading={isLoading}
        className={s.icon}
      />

      {isOffline && (
        <Loaf size="small" color="error">
          OFFLINE
        </Loaf>
      )}

      {pinned && (
        <Title title="Staff pick">
          <div className={s.pin}>
            {Icons.serversStaffPick}
          </div>
        </Title>
      )}

      <div className={s.title}>
        <ServerTitle
          title={server.projectName || server.hostname}
        />

        {!!description && (
          <>
            {descriptionUnderName && <br />}
            <span className={s.description}>
              {description}
            </span>
          </>
        )}
      </div>

      {showTags && (
        <Tags server={server} />
      )}

      {!hideActions && (
        <div className={clsx(s.actions, s['show-on-hover'])}>
          <ServerPower server={server} />

          {Boolean(ServersBoostService) && (
            <ServerBoostButton server={server} />
          )}
        </div>
      )}

      <LastConnectedAt id={server.id} />

      {showDecorator && (
        <div className={clsx(s.decorator)}>
          {showPremiumBadge && (
            <PremiumBadge level={server.premium as any} />
          )}

          <CountryFlag
            forceShow
            locale={server.locale}
            country={server.localeCountry}
          />
        </div>
      )}

      <Favorite id={server.id} />

      <div className={s.players}>
        <ServerPlayersCount server={server} />
      </div>

      {/* <Density server={server} /> */}

      {/* SPACER */}<div />
    </div>
  );
});

const Tags = observer(function Tags({ server }: { server: IServerView }) {
  const ServersService = useService(IServersService);

  return (
    <div className={clsx(s.tags, s['hide-on-hover'])}>
      {ServersService.getTagsForServer(server).map((tag, i) => (
        <Loaf key={tag + i} size="small">
          {tag}
        </Loaf>
      ))}
    </div>
  );
});

function Density({ server }: { server: IServerView }) {
  const density = 0 + Math.round((server.playersCurrent || 0) / (server.playersMax || 1) * 100);

  return (
    <div className={s.density} style={{ '--density': `${density}%` } as any}>
      {Math.ceil((server.playersCurrent || 0) / (server.playersMax || 1) * 100)}&nbsp;
      <span className={s.dim}>%</span>
    </div>
  );
}

const Favorite = observer(function Favorite({ id }: { id: string }) {
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
        icon={
          isInFavoriteServersList
            ? Icons.favoriteActive
            : Icons.favoriteInactive
        }
        onClick={stopPropagation(handleClick)}
      />
    </div>
  );
});

const LastConnectedAt = observer(function LastConnectedAt({ id }: { id: string }) {
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
        {/* <BsClock />
        &nbsp; */}
        last played: {distanceDate}
      </Loaf>
    </Title>
  );
});
