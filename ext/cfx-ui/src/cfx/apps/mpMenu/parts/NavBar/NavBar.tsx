import React from "react";
import { Button, ButtonTheme } from "cfx/ui/Button/Button";
import { ButtonBar } from "cfx/ui/Button/ButtonBar";
import { Icons } from "cfx/ui/Icons";
import { observer } from "mobx-react-lite";
import { Title } from "cfx/ui/Title/Title";
import { UserBar } from "./UserBar/UserBar";
import { $L } from "cfx/common/services/intl/l10n";
import { NavBarState } from "./NavBarState";
import { Badge } from "cfx/ui/Badge/Badge";
import { HomeButton } from "./HomeButton/HomeButton";
import { Flex } from "cfx/ui/Layout/Flex/Flex";
import { Box } from "cfx/ui/Layout/Box/Box";
import { useService } from "cfx/base/servicesContainer";
import { ISettingsUIService } from "cfx/common/services/settings/settings.service";
import { IChangelogService } from "../../services/changelog/changelog.service";
import { Exitter } from "./Exitter/Exitter";

export const NavBar = observer(function NavBar() {
  const ChangelogService = useService(IChangelogService);
  const SettingsUIService = useService(ISettingsUIService);

  React.useEffect(() => {
    NavBarState.setReady();

    return NavBarState.setNotReady;
  }, []);

  const buttonTheme: ButtonTheme = NavBarState.forceTransparentNav
    ? 'default'
    : 'default-blurred';

  return (
    <Flex repell centered gap="large">
      {
        NavBarState.homeButtonVisible
          ? (
            <HomeButton />
          )
          : null
      }

      <Box grow ref={NavBarState.outletRef} />

      <UserBar />

      <ButtonBar>
        <Title title={$L('#Changelogs')}>
          <Button
            to="/changelog"
            size="large"
            theme={buttonTheme}
            icon={Icons.changelog}
            decorator={
              ChangelogService.unreadVersionsCount
                ? (
                  <Badge>
                    {ChangelogService.unreadVersionsCount}
                  </Badge>
                )
                : null
            }
          />
        </Title>
        <Title title={$L('#BottomNav_Settings')}>
          <Button
            size="large"
            theme={buttonTheme}
            icon={Icons.settings}
            onClick={() => SettingsUIService.open()}
          />
        </Title>
      </ButtonBar>

      <Exitter />
    </Flex>
  );
});
