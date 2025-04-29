import { Flex } from "cfx/ui/Layout/Flex/Flex";
import { observer } from "mobx-react-lite";
import { FiServer } from "react-icons/fi";
import { Text } from "cfx/ui/Text/Text";
import { currentGameNameIs } from "cfx/base/gameRuntime";
import { GameName } from "cfx/base/game";
import s from './ExtraLinkyTiles.module.scss';

export const ExtraLinkyTiles = observer(function ExtraLinkyTiles() {
  return (
    <Flex fullWidth gap="large">
      <StartYourServerPromo />

      <a href="https://docs.fivem.net/docs/server-manual/setting-up-a-server/" className={s.tile}>
        <Flex gap="large">
          <div className={s.icon}>
            <FiServer />
          </div>

          <Flex vertical gap="small">
            <Text size="xlarge" weight="bold" family="secondary" opacity="75">
              Host a server
            </Text>

            <Text opacity="50">
              Find out how to host a server on hardware you control
            </Text>
          </Flex>
        </Flex>
      </a >
    </Flex >
  );
});

const zapLogoImageURL = new URL('assets/images/zap.jpg', import.meta.url).toString();

export const StartYourServerPromo = observer(function StartYourServerPromo() {
  const isFiveM = currentGameNameIs(GameName.FiveM);
  const isRedM = currentGameNameIs(GameName.RedM);

  if (!isFiveM && !isRedM) {
    return null;
  }

  const link = isFiveM
    ? 'https://zap-hosting.com/fivemsl'
    : 'https://zap-hosting.com/redm2';

  return (
    <a href={link} className={s.tile}>
      <Flex gap="large">
        <div className={s.icon}>
          <img src={zapLogoImageURL} />
        </div>

        <Flex vertical gap="small">
          <Text size="xlarge" weight="bold" family="secondary" opacity="75">
            Start a server
          </Text>

          <Text opacity="50">
            Rent a server at ZAP-Hosting
          </Text>
        </Flex>
      </Flex>
    </a>
  );
});
