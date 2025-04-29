import { usePlatformStatusService } from "cfx/apps/mpMenu/services/platformStatus/platformStatus.service";
import { StatusLevel } from "cfx/apps/mpMenu/services/platformStatus/types";
import { GameName } from "cfx/base/game";
import { CurrentGameName } from "cfx/base/gameRuntime";
import { $L } from "cfx/common/services/intl/l10n";
import { LinkButton } from "cfx/ui/Button/LinkButton";
import { Icons } from "cfx/ui/Icons";
import { Flex } from "cfx/ui/Layout/Flex/Flex";
import { TextBlock } from "cfx/ui/Text/Text";
import { Title } from "cfx/ui/Title/Title";
import { observer } from "mobx-react-lite";

const statusIconColors = {
  [StatusLevel.AllSystemsOperational]: 'success',
  [StatusLevel.MinorOutage]: 'warning',
  [StatusLevel.MajorOutage]: 'error',
};

const statusIcons = {
  [StatusLevel.AllSystemsOperational]: Icons.statusLevelAllGood,
  [StatusLevel.MinorOutage]: Icons.statusLevelMinor,
  [StatusLevel.MajorOutage]: Icons.statusLevelMajor,
};

export const PlatformStats = observer(function PlatformStats() {
  const PlatformStatusService = usePlatformStatusService();

  const showStatusButton = PlatformStatusService.level > StatusLevel.Unavailable && CurrentGameName !== GameName.RedM;

  return (
    <Flex>
      {showStatusButton && (
        <Title fixedOn="bottom" title={PlatformStatusService.message}>
          <LinkButton
            to="https://status.cfx.re/"
            theme="transparent"
            size="large"
            icon={statusIcons[PlatformStatusService.level]}
            className={`cfx-color-${statusIconColors[PlatformStatusService.level]}`}
          />
        </Title>
      )}

      {PlatformStatusService.hasStats && (
        <Flex vertical centered gap="small">
          <TextBlock opacity="50">
            <Title fixedOn="right" title={$L('#Status_CurrentCount')}>
              <Flex centered gap="small">
                {Icons.playersCount}
                {PlatformStatusService.stats.current}k
              </Flex>
            </Title>
          </TextBlock>
          <TextBlock opacity="50">
            <Title fixedOn="right" title={$L('#Status_PlayerPeak')}>
              <Flex centered gap="small">
                {Icons.last24h}
                {PlatformStatusService.stats.last24h}k
              </Flex>
            </Title>
          </TextBlock>
        </Flex>
      )}
    </Flex>
  );
});
