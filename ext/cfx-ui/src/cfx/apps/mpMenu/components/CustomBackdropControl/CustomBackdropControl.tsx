import { getCustomInGameBackdropPath } from "cfx/apps/mpMenu/parts/ThemeManager/backdrop";
import { useService } from "cfx/base/servicesContainer";
import { Button } from "cfx/ui/Button/Button";
import { Icons } from "cfx/ui/Icons";
import { Flex } from "cfx/ui/Layout/Flex/Flex";
import { Title } from "cfx/ui/Title/Title";
import { observer } from "mobx-react-lite";
import { mpMenu } from "../../mpMenu";
import { useUiService } from "cfx/common/services/ui/ui.service";
import { useConvarService } from "../../services/convars/convars.service";
import s from './CustomBackdropControl.module.scss';

export const CustomBackdropControl = observer(() => {
  const UiService = useUiService();
  const ConvarService = useConvarService();

  const aspectRatio = (UiService.viewportWidth / UiService.viewportHeight).toString();

  const backdropPath = ConvarService.get('ui_customBackdrop');
  const hasCustomBackdropSet = !!backdropPath;

  const onClick = async () => {
    const filePath = await mpMenu.selectFile('backdrop');

    ConvarService.set('ui_customBackdrop', filePath);
  };

  return (
    <Flex vertical>
      <Flex>
        <Button
          text="Select"
          onClick={onClick}
        />

        {hasCustomBackdropSet && (
          <Title fixedOn="right" title="Delete custom menu backdrop">
            <Button
              theme="transparent"
              icon={Icons.remove}
              onClick={() => ConvarService.set('ui_customBackdrop', '')}
            />
          </Title>
        )}
      </Flex>

      <div
        className={s.preview}
        style={{
          aspectRatio,
          backgroundImage: `url(${getCustomInGameBackdropPath(backdropPath)})`,
        }}
      />

      {/* {hasCustomBackdropSet && (
      )} */}
    </Flex>
  );
});
