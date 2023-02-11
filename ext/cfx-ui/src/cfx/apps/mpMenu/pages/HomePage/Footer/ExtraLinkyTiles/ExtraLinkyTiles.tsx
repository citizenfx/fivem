import { Flex } from "cfx/ui/Layout/Flex/Flex";
import { clsx } from "cfx/utils/clsx";
import { observer } from "mobx-react-lite";
import { FiServer } from "react-icons/fi";
import { Text } from "cfx/ui/Text/Text";
import { currentGameNameIs } from "cfx/base/gameRuntime";
import { GameName } from "cfx/base/game";
import s from './ExtraLinkyTiles.module.scss';

interface ExtraLinkyTileCustomProps {
  growHeight?: boolean,
}

const zapLogoImageURL = new URL('assets/images/zap.jpg', import.meta.url).toString();

export const ExtraLinkyTileStartServer = observer(function ExtraLinkyTileStartServer(props: ExtraLinkyTileCustomProps) {
  const isFiveM = currentGameNameIs(GameName.FiveM);
  const isRedM = currentGameNameIs(GameName.RedM);

  if (!isFiveM && !isRedM) {
    return null;
  }

  const link = isFiveM
    ? 'https://zap-hosting.com/fivemsl'
    : 'https://zap-hosting.com/redm2';

  return (
    <ExtraLinkyTile
      title="Start a server"
      description="Rent a server at ZAP-Hosting"
      link={link}
      icon={<img src={zapLogoImageURL} />}
      growHeight={props.growHeight}
    />
  );
});

export const ExtraLinkyTileHostServer = observer(function ExtraLinkyTileHostServer(props: ExtraLinkyTileCustomProps) {
  return (
    <ExtraLinkyTile
      title="Host a server"
      description="Find out how to host a server on hardware you control"
      link="https://docs.fivem.net/docs/server-manual/setting-up-a-server/"
      icon={<FiServer />}
      growHeight={props.growHeight}
    />
  );
});


interface ExtraLinkyTileProps {
  title: string,
  description: string,
  link: string,
  icon: React.ReactNode,
  growHeight?: boolean,
}

const ExtraLinkyTile = observer(function StartYourServerPromo(props: ExtraLinkyTileProps) {
  const className = clsx(s.tile, {
    [s.growHeight]: props.growHeight,
  });

  return (
    <a href={props.link} className={className}>
      <Flex gap="large">
        <div className={s.icon}>
          {props.icon}
        </div>

        <Flex vertical gap="small">
          <Text size="xlarge" weight="bold" family="secondary" opacity="75">
            {props.title}
          </Text>

          <Text opacity="50">
            {props.description}
          </Text>
        </Flex>
      </Flex>
    </a>
  );
});
