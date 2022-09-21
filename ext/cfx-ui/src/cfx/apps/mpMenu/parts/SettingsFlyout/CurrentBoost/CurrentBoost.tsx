import { ServerTileItem } from "cfx/common/parts/Server/ServerTileItem/ServerTileItem";
import { useAccountService } from "cfx/common/services/account/account.service";
import { $L } from "cfx/common/services/intl/l10n";
import { useServersService } from "cfx/common/services/servers/servers.service";
import { useServersBoostService } from "cfx/common/services/servers/serversBoost.service";
import { Indicator } from "cfx/ui/Indicator/Indicator";
import { InfoPanel } from "cfx/ui/InfoPanel/InfoPanel";
import { observer } from "mobx-react-lite";

export function isCurrentBoostVisible() {
  // const ServersService = useServersService();
  const AccountService = useAccountService();
  // const ServersBoostService = useServersBoostService();

  const hasAccount = !!AccountService.account;
  // const accountLoading = !AccountService.accountLoadComplete;

  // const hasBoost = !!ServersBoostService.currentBoost;
  // const boostLoading = !ServersBoostService.currentBoostLoadComplete;

  return hasAccount;
}

export const CurrentBoost = observer(function CurrentBoost() {
  const ServersService = useServersService();
  const ServersBoostService = useServersBoostService();

  const hasBoost = !!ServersBoostService.currentBoost;
  const boostLoading = !ServersBoostService.currentBoostLoadComplete;
  const boostLoadingError = ServersBoostService.currentBoostLoadError;

  if (boostLoading) {
    return (
      <Indicator />
    );
  }

  if (!hasBoost) {
    return $L('#Settings_BoostNone');
  }

  if (boostLoadingError) {
    return (
      <InfoPanel type="error">
        {boostLoadingError}
      </InfoPanel>
    );
  }

  const server = ServersService.getServer(ServersBoostService.currentBoost.address);
  if (!server) {
    return (
      <InfoPanel type="warning">
        {$L('#Settings_BoostServerMissing', { address: ServersBoostService.currentBoost.address })}
      </InfoPanel>
    );
  }

  return (
    <ServerTileItem
      server={server}
    />
  );
});
