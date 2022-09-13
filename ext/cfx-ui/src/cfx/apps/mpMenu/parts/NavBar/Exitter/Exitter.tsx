import { mpMenu } from "cfx/apps/mpMenu/mpMenu";
// import { playSfx, Sfx } from "cfx/apps/mpMenu/utils/sfx";
import { $L } from "cfx/common/services/intl/l10n";
import { Button, ButtonTheme } from "cfx/ui/Button/Button";
import { Icons } from "cfx/ui/Icons";
import { Flex } from "cfx/ui/Layout/Flex/Flex";
import { AfterTitleOutlet } from "cfx/ui/outlets";
import { Text } from "cfx/ui/Text/Text";
import { Title } from "cfx/ui/Title/Title";
import { useOpenFlag } from "cfx/utils/hooks";
// import { useEffect } from "react";
import ReactFocusLock from "react-focus-lock";
import { NavBarState } from "../NavBarState";
import s from './Exitter.module.scss';

export function Exitter() {
  const [confirmerOpen, openConfirmer, closeConfirmer] = useOpenFlag(false);

  // useEffect(() => {
  //   if (confirmerOpen) {
  //     playSfx(Sfx.Maybe);
  //   }
  // }, [confirmerOpen]);

  const buttonTheme: ButtonTheme = NavBarState.forceTransparentNav
    ? 'default'
    : 'default-blurred';

  return (
    <>
      <Title fixedOn="bottom-right" title={$L('#ExitToDesktop')}>
        <Button
          size="large"
          icon={Icons.exit}
          theme={buttonTheme}
          onClick={openConfirmer}
        />
      </Title>

      {confirmerOpen && (
        <AfterTitleOutlet>
          <div className={s.root}>
            <div className={s.wrapper}>
              <ReactFocusLock disabled={!confirmerOpen}>
                <Flex centered="axis">
                  <Text size="xxlarge">
                    Are you sure?
                  </Text>

                  <Button
                    theme="default-blurred"
                    size="large"
                    text={$L('#ExitToDesktop')}
                    onClick={() => mpMenu.invokeNative('exit')}
                  />
                  <Button
                    theme="primary"
                    size="large"
                    text="Cancel"
                    onClick={closeConfirmer}
                  />
                </Flex>
              </ReactFocusLock>
            </div>
          </div>
        </AfterTitleOutlet>
      )}
    </>
  );
}
