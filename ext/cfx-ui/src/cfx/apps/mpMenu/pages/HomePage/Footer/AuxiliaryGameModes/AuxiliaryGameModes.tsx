import { GameName } from "cfx/base/game";
import { currentGameNameIs } from "cfx/base/gameRuntime";
import { Flex } from "cfx/ui/Layout/Flex/Flex";
import { FlexRestricter } from "cfx/ui/Layout/Flex/FlexRestricter";
import { observer } from "mobx-react-lite";
import { ReplayEditor } from "./ReplayEditor/ReplayEditor";
import { StoryMode } from "./StoryMode/StoryMode";

export const AuxiliaryGameModes = observer(function AuxiliaryGameModes() {
  if (!currentGameNameIs(GameName.FiveM)) {
    return null;
  }

  return (
    <Flex>
      {/* <FlexRestricter> */}
        <StoryMode />
      {/* </FlexRestricter> */}

      {/* <FlexRestricter> */}
        <ReplayEditor />
      {/* </FlexRestricter> */}
    </Flex>
  );
});
