import { KnownConvars, useConvarService } from "cfx/apps/mpMenu/services/convars/convars.service";
import { GameUpdateChannel } from "cfx/base/game";
import { Loaf } from "cfx/ui/Loaf/Loaf";
import { Title } from "cfx/ui/Title/Title";
import { observer } from "mobx-react-lite";

const titleNode = (
  <>
    🚧 The design of this screen is still in development.
    <br />
    <br />
    We're happy to hear your feedback in our official Discord (Chat button on the right)!
  </>
);

export const BetaSign = observer(function BetaSign() {
  const ConvarService = useConvarService();

  if (ConvarService.get(KnownConvars.updateChannel) !== GameUpdateChannel.Canary) {
    return null;
  }

  return (
    <Title fixedOn="bottom-left" title={titleNode}>
      <Loaf size="large" color="error">
        WIP UI
      </Loaf>
    </Title>
  );
});
