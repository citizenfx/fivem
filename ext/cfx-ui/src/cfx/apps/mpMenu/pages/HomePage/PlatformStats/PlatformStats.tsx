import {
  Icons,
  Flex,
  TextBlock,
  Title,
} from '@cfx-dev/ui-components';
import { observer } from 'mobx-react-lite';

import { usePlatformStatusService } from 'cfx/apps/mpMenu/services/platformStatus/platformStatus.service';
import { $L } from 'cfx/common/services/intl/l10n';

export const PlatformStats = observer(function PlatformStats() {
  const PlatformStatusService = usePlatformStatusService();

  if (!PlatformStatusService.hasStats) {
    return null;
  }

  return (
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
  );
});
