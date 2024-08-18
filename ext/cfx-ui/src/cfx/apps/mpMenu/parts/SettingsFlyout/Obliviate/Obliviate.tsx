import { Button } from '@cfx-dev/ui-components';
import { observer } from 'mobx-react-lite';

import { $L } from 'cfx/common/services/intl/l10n';
import { useServersService } from 'cfx/common/services/servers/servers.service';

export const Obliviate = observer(function Obliviate() {
  const ServersService = useServersService();

  const historyList = ServersService.getHistoryList();

  if (!historyList || historyList.sequence.length === 0) {
    return null;
  }

  return (
    <Button text={$L('#Settings_ClearHistory')} onClick={historyList.clear} />
  );
});
