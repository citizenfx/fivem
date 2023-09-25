import { ButtonBar } from "cfx/ui/Button/ButtonBar";
import { LinkButton } from "cfx/ui/Button/LinkButton";
import { Title } from "cfx/ui/Title/Title";
import { returnTrue } from "cfx/utils/functional";
import { observer } from "mobx-react-lite";
import { currentGameNameIs } from "cfx/base/gameRuntime";
import { GameName } from "cfx/base/game";
import { $L } from "cfx/common/services/intl/l10n";

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
    label: 'Discord',
    visible: returnTrue,
  },
];

export const HomePageNavBarLinks = observer(function HomePageLinks() {
  const linkNodes = homePageNavBarLinks
    .filter(({ visible }) => visible())
    .map(({ href, label }) => (
      <Title key={href} title={$L('#Global_OpenLinkInBrowser')}>
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
