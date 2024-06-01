import { ButtonBar } from "cfx/ui/Button/ButtonBar";
import { AnalyticsLinkButton } from "cfx/common/parts/AnalyticsLinkButton/AnalyticsLinkButton";
import { Title } from "cfx/ui/Title/Title";
import { returnTrue } from "cfx/utils/functional";
import { observer } from "mobx-react-lite";
import { currentGameNameIs } from "cfx/base/gameRuntime";
import { GameName } from "cfx/base/game";
import { $L } from "cfx/common/services/intl/l10n";
import { useEventHandler } from "cfx/common/services/analytics/analytics.service";
import { EventActionNames, ElementPlacements } from "cfx/common/services/analytics/types";
import React from "react";

const enum IHomePageNavBarLinkIDs {
  FiveM,
  RedM,
  Forum,
  Patreon,
  Discord,
};

interface IHomePageNavBarLink {
  id: IHomePageNavBarLinkIDs,
  href: string,
  label: string,
  visible(): boolean,
}

const homePageNavBarLinks: IHomePageNavBarLink[] = [
  {
    id: IHomePageNavBarLinkIDs.FiveM,
    href: 'https://fivem.net',
    label: 'FiveM.net',
    visible: () => currentGameNameIs(GameName.FiveM),
  },
  {
    id: IHomePageNavBarLinkIDs.RedM,
    href: 'https://redm.net',
    label: 'RedM.net',
    visible: () => currentGameNameIs(GameName.RedM),
  },
  {
    id: IHomePageNavBarLinkIDs.Forum,
    href: 'https://forum.cfx.re',
    label: 'Forum',
    visible: returnTrue,
  },
  {
    id: IHomePageNavBarLinkIDs.Patreon,
    href: 'https://patreon.com/fivem',
    label: 'Patreon',
    visible: returnTrue,
  },
  {
    id: IHomePageNavBarLinkIDs.Discord,
    href: 'https://discord.gg/fivem',
    label: 'Discord',
    visible: returnTrue,
  },
];

export const HomePageNavBarLinks = observer(function HomePageLinks() {
  const eventHandler = useEventHandler();

  const handleForumClick = React.useCallback(() => {
    const forumItem = homePageNavBarLinks.find(({ id }) => id === IHomePageNavBarLinkIDs.Forum);

    if (!forumItem) {
      return;
    }

    eventHandler({ action: EventActionNames.ForumCTA, properties: {
      element_placement: ElementPlacements.Nav,
      text: forumItem.label,
      link_url: forumItem.href,
    }});
  }, [eventHandler]);

  const linkNodes = homePageNavBarLinks
    .filter(({ visible }) => visible())
    .map(({ href, label, id }) => (
      <Title key={id} title={$L('#Global_OpenLinkInBrowser')}>
        <AnalyticsLinkButton
          to={href}
          text={label}
          size="large"
          theme="transparent"
          elementPlacement={ElementPlacements.Nav}
          onClick={id === IHomePageNavBarLinkIDs.Forum ? handleForumClick : undefined}
        />
      </Title>
    ));

  return (
    <ButtonBar>
      {linkNodes}
    </ButtonBar>
  );
});
