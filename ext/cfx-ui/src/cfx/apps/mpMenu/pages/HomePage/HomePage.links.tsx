import React from "react";
import { ButtonBar } from "cfx/ui/Button/ButtonBar";
import { LinkButton } from "cfx/ui/Button/LinkButton";
import { Flex } from "cfx/ui/Layout/Flex/Flex";
import { Title } from "cfx/ui/Title/Title";
import { returnTrue } from "cfx/utils/functional";
import { observer } from "mobx-react-lite";
import { CurrentGameName, currentGameNameIs } from "cfx/base/gameName";
import { GameName } from "cfx/base/game";
import { Icons } from "cfx/ui/Icons";
import { usePlatformStatusService } from "../../services/platformStatus/platformStatus.service";
import { StatusLevel } from "../../services/platformStatus/types";
import { BsCloudPlus } from "react-icons/bs";
import { FiServer } from "react-icons/fi";
import { IoSparklesOutline } from "react-icons/io5";

interface IHomePageNavBarLink {
  href: string,
  label: string,
  visible(): boolean,
}

const homePageNavBarLinks: IHomePageNavBarLink[] = [
  {
    href: 'https://fivem.net',
    label: 'FiveM.net',
    visible: () => currentGameNameIs(GameName.FiveM),
  },
  {
    href: 'https://redm.net',
    label: 'RedM.net',
    visible: () => currentGameNameIs(GameName.RedM),
  },
  {
    href: 'https://forum.cfx.re',
    label: 'Forum',
    visible: returnTrue,
  },
  {
    href: 'https://patreon.com/fivem',
    label: 'Patreon',
    visible: returnTrue,
  },
  {
    href: 'https://discord.gg/fivem',
    label: 'Chat',
    visible: () => usePlatformStatusService().is(StatusLevel.AllSystemsOperational),
  },
];

export const HomePageNavBarLinks = observer(function HomePageLinks() {
  const linkNodes = homePageNavBarLinks
    .filter(({ visible }) => visible())
    .map(({ href, label }) => (
      <Title key={href} title="Opens in browser">
        <LinkButton
          to={href}
          text={label}
          size="large"
          theme="transparent"
        />
      </Title>
    ));

  return (
    <ButtonBar>
      {linkNodes}
    </ButtonBar>
  );
});

interface IHomePageBottomLink {
  href: string,
  label: string,
  title: string,
  icon?: React.ReactNode,
  visible?(): boolean,
}

const homePageBottomLinks: IHomePageBottomLink[] = [
  {
    href: 'https://zap-hosting.com/fivemsl',
    label: 'Start a server',
    title: 'Rent a server at ZAP-Hosting',
    icon: <BsCloudPlus />,
    visible: () => currentGameNameIs(GameName.FiveM),
  },
  {
    href: 'https://zap-hosting.com/redm2',
    label: 'Start a server',
    title: 'Rent a server at ZAP-Hosting',
    icon: <BsCloudPlus />,
    visible: () => currentGameNameIs(GameName.RedM),
  },
  {
    href: 'https://docs.fivem.net/docs/server-manual/setting-up-a-server/',
    label: 'Host a server',
    title: 'Find out how to host a server on hardware you control',
    icon: <FiServer />,
  },
  {
    href: 'https://docs.fivem.net/docs/scripting-manual/introduction/',
    label: 'Make mods',
    title: 'Learn how to create your own customized content for your server or game!',
    icon: <IoSparklesOutline />,
  },
  {
    href: 'https://docs.fivem.net/docs/contributing/how-you-can-help/',
    label: 'Contribute to the project',
    title: 'Learn how to contribute to the FiveM source code',
  },
];

export const HomePageBottomLinks = observer(function HomePageBottomLinks() {
  const linkNodes = homePageBottomLinks
    .filter(({ visible }) => visible ? visible() : true)
    .map(({ href, label, title, icon }) => (
      <Title key={href} title={title} fixedOn="top">
        <LinkButton
          to={href}
          text={label}
          icon={icon}
          size="large"
          theme="default-blurred"
        />
      </Title>
    ));

  return (
    <Flex>
      {linkNodes}
    </Flex>
  );
});
