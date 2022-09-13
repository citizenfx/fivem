import { Flex } from "cfx/ui/Layout/Flex/Flex";
import { observer } from "mobx-react-lite";
import { FiServer } from "react-icons/fi";
import { IoSparklesOutline } from "react-icons/io5";
// import { HomePageBottomLinks } from "../HomePage.links";
import { StartYourServerPromo } from "./StartYourServerPromo/StartYourServerPromo";
import s from './ExtraLinkyTiles.module.scss';
import { Text } from "cfx/ui/Text/Text";
import { Icon } from "cfx/ui/Icon/Icon";
import { clsx } from "cfx/utils/clsx";
import { ui } from "cfx/ui/ui";
import { LinkButton } from "cfx/ui/Button/LinkButton";
import { Title } from "cfx/ui/Title/Title";
import { Box } from "cfx/ui/Layout/Box/Box";

export const ExtraLinkyTiles = observer(function ExtraLinkyTiles() {
  return (
    <Flex fullWidth fullHeight vertical repell gap="large">
      <Flex vertical fullWidth gap="large">
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
      </Flex>

      <Flex centered>
        <Title title="Learn how to create your own customized content for your server or game!">
          <LinkButton
            to="https://docs.fivem.net/docs/scripting-manual/introduction/"
            text="Make mods"
            size="large"
            icon={<IoSparklesOutline />}
            theme="default-blurred"
          />
        </Title>

        <Title title="Learn how to contribute to the FiveM source code">
          <LinkButton
            to="https://docs.fivem.net/docs/contributing/how-you-can-help/"
            text="Contribute to the project"
            size="large"
            theme="default-blurred"
          />
        </Title>
      </Flex>
    </Flex >
  );
});
