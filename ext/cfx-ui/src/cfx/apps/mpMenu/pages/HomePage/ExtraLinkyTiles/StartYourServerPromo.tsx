import { GameName } from "cfx/base/game";
import { currentGameNameIs } from "cfx/base/gameName";
import { Flex } from "cfx/ui/Layout/Flex/Flex";
import { Text } from "cfx/ui/Text/Text";
import { observer } from "mobx-react-lite";
import s from './ExtraLinkyTiles.module.scss';

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
