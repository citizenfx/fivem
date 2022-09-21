import React from "react";
import { MAX_TOP_SERVERS, HomeScreenServerListService, getTopRegionServerOnScreenTime } from "cfx/apps/mpMenu/services/servers/list/HomeScreenServerList.service";
import { useService } from "cfx/base/servicesContainer";
import { ServerConnectButton } from "cfx/common/parts/Server/ServerConnectButton/ServerConnectButton";
import { ServerCoreLoafs } from "cfx/common/parts/Server/ServerCoreLoafs/ServerCoreLoafs";
import { ServerFavoriteButton } from "cfx/common/parts/Server/ServerFavoriteButton/ServerFavoriteButton";
import { ServerIcon } from "cfx/common/parts/Server/ServerIcon/ServerIcon";
import { ServerPlayersCount } from "cfx/common/parts/Server/ServerPlayersCount/ServerPlayersCount";
import { ServerPower } from "cfx/common/parts/Server/ServerPower/ServerPower";
import { ServerTitle } from "cfx/common/parts/Server/ServerTitle/ServerTitle";
import { useServersService } from "cfx/common/services/servers/servers.service";
import { IServerView } from "cfx/common/services/servers/types";
import { Icons } from "cfx/ui/Icons";
import { Flex } from "cfx/ui/Layout/Flex/Flex";
import { FlexRestricter } from "cfx/ui/Layout/Flex/FlexRestricter";
import { Text, TextBlock } from "cfx/ui/Text/Text";
import { ui } from "cfx/ui/ui";
import { clsx } from "cfx/utils/clsx";
import { makeAutoObservable } from "mobx";
import { observer } from "mobx-react-lite";
import { PremiumBadge } from "cfx/ui/PremiumBadge/PremiumBadge";
import { clamp } from "cfx/utils/math";
import { Loaf } from "cfx/ui/Loaf/Loaf";
import { ControlBox } from "cfx/ui/ControlBox/ControlBox";
import { useNavigate } from "react-router-dom";
import { getServerDetailsLink } from "cfx/common/parts/Server/ServerListItem/utils";
import s from './TopServers.module.scss';

export const TopServers = observer(function TopServers() {
  React.useEffect(() => {
    Ctrl.reset();

    return () => {
      Ctrl.pause();
    };
  }, []);

  const PersonalizedServerList = useService(HomeScreenServerListService);

  const sequence = PersonalizedServerList.topRegionServers;

  const selectorNodes: React.ReactNode[] = [];
  const cardNodes: React.ReactNode[] = [];

  sequence.forEach((server, index) => {
    const active = Ctrl.activeIndex === index;

    selectorNodes.push(<Item key={server.id} active={active} server={server} index={index} />);
    cardNodes.push(<Card key={server.id} active={active} server={server} />);
  });

  return (
    <div className={s.root}>
      <div className={s.selector}>
        {selectorNodes}
      </div>

      <div
        className={s.cardHolder}
        onMouseOver={Ctrl.pause}
        onMouseLeave={Ctrl.unpause}
      >
        {cardNodes}
      </div>
    </div>
  );
});

interface ItemProps {
  server: IServerView,
  active: boolean,
  index: number,
}
const Item = observer(function Item(props: ItemProps) {
  const {
    server,
    active,
    index,
  } = props;

  const navigate = useNavigate();

  const handleClick = React.useCallback(() => {
    if (active) {
      navigate(getServerDetailsLink(server));

      return;
    }

    Ctrl.setActiveIndex(index);
  }, [navigate, server, active, index]);

  const itemClassName = clsx(s.item, {
    [s.active]: active,
  });

  return (
    <div
      onClick={handleClick}
      className={itemClassName}
    >
      <ServerIcon
        glow={active}
        type="list"
        server={server}
        className={s.icon}
      />

      <div className={s.title}>
        <ServerTitle
          truncated
          size="large"
          title={server.projectName || server.hostname}
        />
      </div>

      {active && (
        <Progress />
      )}
    </div>
  );
});

interface CardProps {
  server: IServerView,
  active: boolean,
}
const Card = observer(function Card(props: CardProps) {
  const {
    server,
    active,
  } = props;

  const navigate = useNavigate();
  const ServersService = useServersService();

  const handleClick = React.useCallback(() => {
    navigate(getServerDetailsLink(server));
  }, [navigate, server]);

  const tagNodes = !!server.tags
    ? ServersService.getTagsForServer(server).map((tag, i) => (
      <Loaf key={tag + i} size="small">
        {tag}
      </Loaf>
    ))
    : undefined;

  const cardStyle = {
    '--banner': ui.url(server.bannerDetail),
  } as any;

  const cardClassName = clsx(s.card, {
    [s.active]: active,
  });

  return (
    <div
      style={cardStyle}
      onClick={handleClick}
      className={cardClassName}
    >
      <div className={s.background} />

      <div className={s.hoverDecoration} />

      <div className={s.content}>
        <Flex vertical fullWidth fullHeight>
          <FlexRestricter vertical>
            <Flex fullHeight gap="xlarge">
              <Flex vertical>
                <ServerIcon
                  glow
                  type="details"
                  server={server}
                  className={s.icon}
                />

                <Flex centered>
                  {!!server.premium && (
                    <PremiumBadge level={server.premium} />
                  )}

                  <ServerPower server={server} />
                </Flex>

                <Flex centered>
                  <Text opacity="75">
                    {Icons.playersCount}
                  </Text>
                  <Text opacity="75">
                    <ServerPlayersCount server={server} />
                  </Text>
                </Flex>
              </Flex>

              <FlexRestricter>
                <Flex vertical fullHeight>
                  <ServerTitle
                    truncated
                    size="xxxlarge"
                    title={server.projectName || server.hostname}
                  />

                  <div className={s.description}>
                    <TextBlock typographic opacity="75">
                      {server.projectDescription}
                    </TextBlock>
                  </div>
                </Flex>
              </FlexRestricter>
            </Flex>
          </FlexRestricter>

          <Flex repell alignToEndAxis>
            <Flex>
              <ServerConnectButton server={server} />
              <ServerFavoriteButton theme="transparent" server={server} />
            </Flex>

            <Flex vertical alignToEndAxis>
              {!!tagNodes?.length && (
                <Flex centered="axis">
                  <Text asDiv typographic opacity="50">
                    <ControlBox>
                      {Icons.tags}
                    </ControlBox>
                  </Text>

                  {tagNodes}
                </Flex>
              )}

              <Flex>
                <ServerCoreLoafs hideActions server={server} />
              </Flex>
            </Flex>
          </Flex>
        </Flex>
      </div>
    </div>
  );
});

const Progress = observer(function Progress() {
  return (
    <div
      ref={Ctrl.reffer}
      className={s.progress}
    />
  );
});

const Ctrl = new class Ctrl {
  private _activeIndex: number = 0;
  public get activeIndex(): number { return this._activeIndex }
  private set activeIndex(activeIndex: number) { this._activeIndex = activeIndex }

  private progress = 0;
  private lastDt = 0;
  private paused = false;

  private rAF: RequestAnimationFrameReturn | null = null;

  private $activeProgressElement: HTMLDivElement | null = null;

  constructor() {
    makeAutoObservable(this, {
      // @ts-expect-error private
      progress: false,
      lastDt: false,
      paused: false,
      draw: false,
      rAF: false,
      $activeProgressElement: false,
    });

    this.draw();
  }

  setActiveIndex(index: number) {
    this.progress = 0;
    this.activeIndex = clamp(index, 0, MAX_TOP_SERVERS - 1);
  }

  readonly reset = () => {
    this.activeIndex = 0;

    this.unpause();
  };

  readonly pause = () => this.paused = true;
  readonly unpause = () => this.paused = false;

  readonly reffer = (ref: HTMLDivElement | null) => {
    // Reset progress
    if (this.$activeProgressElement) {
      this.$activeProgressElement.style.setProperty('--progress', ui.pc(0));
    }

    this.$activeProgressElement = ref;
  };

  private readonly draw = (timer: number = 0) => {
    const dt = timer - this.lastDt;
    this.lastDt = timer;

    this.rAF = requestAnimationFrame(this.draw);

    if (this.paused) {
      return;
    }

    const timeout = getTopRegionServerOnScreenTime(this.activeIndex);

    const advance = (dt / timeout) * 100;

    let progress = clamp(this.progress + advance, 0, 100);
    if (progress === 100) {
      progress = 0;

      let activeIndex = this.activeIndex + 1;
      if (activeIndex === MAX_TOP_SERVERS) {
        activeIndex = 0;
      }

      this.activeIndex = activeIndex;
    }

    this.progress = progress;

    if (this.$activeProgressElement) {
      this.$activeProgressElement.style.setProperty('--progress', ui.pc(progress));
    }
  };
};
