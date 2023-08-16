import { Page } from "cfx/ui/Layout/Page/Page";
import { Flex } from "cfx/ui/Layout/Flex/Flex";
import { InsideNavBar } from "cfx/apps/mpMenu/parts/NavBar/InsideNavBar";
import { observer } from "mobx-react-lite";
import { IAuthFormState } from "cfx/common/parts/AuthForm/AuthFormState";
import { LoginLeftSide } from "./LoginLeftSide/LoginLeftSide";
import { LoginRightSide } from "./LoginRightSide/LoginRightSide";
import s from './LoginContainer.module.scss';

export type LoginFormProps = Partial<Pick<
  IAuthFormState,
  | 'onDone'
  | 'onModeChange'
  | 'onDisabledChange'
>>

export const LoginContainer = observer(function LoginContainer(props: LoginFormProps) {
    return (
        <Flex gap="small" className={s["login-container"]}>
          <LoginLeftSide />
          <LoginRightSide />
        </Flex>
    );
});
