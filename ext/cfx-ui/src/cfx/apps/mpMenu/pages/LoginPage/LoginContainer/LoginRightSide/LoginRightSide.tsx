import { IAuthFormState, totpFieldRef, useAuthFormState } from "cfx/common/parts/AuthForm/AuthFormState";
import { useAccountService } from "cfx/common/services/account/account.service";
import { $L } from "cfx/common/services/intl/l10n";
import { Flex } from "cfx/ui/Layout/Flex/Flex";
import { Text } from "cfx/ui/Text/Text";
import { observer } from "mobx-react-lite";
import { Input } from 'cfx/ui/Input/Input';
import { Button } from "cfx/ui/Button/Button";
import { Indicator } from "cfx/ui/Indicator/Indicator";
import s from './LoginRightSide.module.scss';
import { useLastHistoryServer } from "../../../HomePage/Continuity/LastConnectedTile/LastConnectedTile";
import { useServiceOptional } from "cfx/base/servicesContainer";
import { IServersConnectService } from "cfx/common/services/servers/serversConnect.service";
import { timeout } from "cfx/utils/async";
import React from "react";
import { useNavigate } from "react-router-dom";
import { mpMenu } from "cfx/apps/mpMenu/mpMenu";



export const LoginRightSide = observer(function LoginRightSide() {
  const AccountService = useAccountService();
  const state = useAuthFormState();
  const ServersConnectService = useServiceOptional(IServersConnectService);

  const server = useLastHistoryServer();
  const te: any = $L('#AuthForm_LogIn_Submit');
  const [buttonText, setButtonText] = React.useState(te);
  const navigate = useNavigate();

  const handleSubmit = async () => {

    /* if (!server) {
        return null;
    }
    console.log(server); */
    state.submitPending = true;
    setButtonText("Đang đăng nhập...");


    await timeout(1000);

    mpMenu.invokeNative('setDS_Identifier', state.email.value);

    navigate(`/devoted`);

    // ServersConnectService?.connectTo(server);
  };

  return (
    <Flex vertical centered gap="large" className={s["right-side"]}>
      <Flex centered className={s["server-information"]}>
        <img className={s.logo} src="https://i.imgur.com/tEwx0RH.png" />
        <div>
          <Text size="xlarge" className={s.title} asDiv>Roleplay</Text>
          <Text size="small" className={s.title} asDiv>Have funny moments</Text>
        </div>
      </Flex>
      {state.showEmailField && (
        <Input
          autofocus
          fullWidth
          label={$L('#AuthForm_FieldLabel_Email')}
          placeholder="devoted@gmail.com"
          value={state.email.value}
          onChange={state.email.set}
          disabled={state.disabled}
          onSubmit={handleSubmit}
          decorator={state.renderEmailDecorator()}
        />
      )}

      {state.showPasswordField && (
        <Input
          fullWidth
          type="password"
          label={$L('#AuthForm_FieldLabel_Password')}
          placeholder="********"
          value={state.password}
          onChange={state.setPassword}
          disabled={state.disabled}
          onSubmit={handleSubmit}

          decorator={state.renderPasswordDecorator()}
        />
      )}

      <SubmitControls state={state} testSubmit={handleSubmit} buttonText={buttonText} />
    </Flex>
  );
});

interface SubmitControlsProps {
  state: IAuthFormState,
  testSubmit: any,
  buttonText: any
}

const SubmitControls = observer(function SubmitControls({ state, testSubmit, buttonText }: SubmitControlsProps) {
  if (state.isLogIn) {
    return (
      <Flex centered repell>
        <Button
          text={buttonText}
          theme={state.disabled ? 'default' : 'primary'}
          disabled={state.disabled || !state.canSubmit}
          onClick={testSubmit}
          icon={state.submitPending ? <Indicator /> : null}
        />
      </Flex>
    );
  }


  return (
    <Flex centered repell>
      <Button
        text={$L('#AuthForm_SwitchTo_LogIn')}
        theme="transparent"
        disabled={state.disabled}
        onClick={state.switchToLogIn}
      />

      {state.disabled ? <Indicator /> : null}

      <Button
        text={$L('#AuthForm_Registration_Submit')}
        theme={state.disabled ? 'default' : 'primary'}
        disabled={state.disabled || !state.canSubmit}
        onClick={testSubmit}
      />
    </Flex>
  );
});
