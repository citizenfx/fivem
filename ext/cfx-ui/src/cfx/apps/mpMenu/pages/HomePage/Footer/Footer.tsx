import { AnalyticsLinkButton } from "cfx/common/parts/AnalyticsLinkButton/AnalyticsLinkButton";
import { Box } from "cfx/ui/Layout/Box/Box";
import { Flex } from "cfx/ui/Layout/Flex/Flex";
import { Title } from "cfx/ui/Title/Title";
import { ui } from "cfx/ui/ui";
import { observer } from "mobx-react-lite";
import { IoSparklesOutline } from "react-icons/io5";
import { AuxiliaryGameModes } from "./AuxiliaryGameModes/AuxiliaryGameModes";
import { ExtraLinkyTiles } from "./ExtraLinkyTiles/ExtraLinkyTiles";
import { FeaturedServerTile, useFeaturedServer } from "./FeaturedServerTile/FeaturedServerTile";
import { ElementPlacements } from "cfx/common/services/analytics/types";

export const Footer = observer(function Footer() {
  const hasFeaturedServer = Boolean(useFeaturedServer());

  return (
    <Flex vertical alignToEnd gap="large" className={ui.cls.flexGrow}>
      <Flex gap="large">
        {!!hasFeaturedServer && (
          <Box width="50%">
            <FeaturedServerTile />
          </Box>
        )}

        <ExtraLinkyTiles />
      </Flex>

      <Flex repell>
        <AuxiliaryGameModes />

        <Flex>
          <Title fixedOn="top" title="Learn how to create your own customized content for your server or game!">
            <AnalyticsLinkButton
              to="https://docs.fivem.net/docs/scripting-manual/introduction/"
              text="Make mods"
              size="large"
              icon={<IoSparklesOutline />}
              theme="default-blurred"
              elementPlacement={ElementPlacements.Footer}
            />
          </Title>

          <Title fixedOn="top" title="Learn how to contribute to the FiveM source code">
            <AnalyticsLinkButton
              to="https://docs.fivem.net/docs/contributing/how-you-can-help/"
              text="Contribute to the project"
              size="large"
              theme="default-blurred"
              elementPlacement={ElementPlacements.Footer}
            />
          </Title>
        </Flex>
      </Flex>
    </Flex>
  );
});
