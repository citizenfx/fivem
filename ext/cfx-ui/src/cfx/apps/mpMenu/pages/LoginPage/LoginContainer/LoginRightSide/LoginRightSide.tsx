import { IAuthFormState, totpFieldRef, useAuthFormState } from "cfx/common/parts/AuthForm/AuthFormState";
import { useAccountService } from "cfx/common/services/account/account.service";
import { $L } from "cfx/common/services/intl/l10n";
import { Flex } from "cfx/ui/Layout/Flex/Flex";
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



export const LoginRightSide = observer(function LoginRightSide() {
    const AccountService = useAccountService();
    const state = useAuthFormState();
    const ServersConnectService = useServiceOptional(IServersConnectService);

    const server = useLastHistoryServer();
    const te:any = $L('#AuthForm_LogIn_Submit');
    const [buttonText, setButtonText] = React.useState(te);

    const handleSubmit = async () => {
    
        if (!server) {
            return null;
        }
        console.log(server);
        setButtonText("Đang đăng nhập...");
        
        await timeout(4000);
        setButtonText("Đang kết nối...");

        await timeout(5000);
        setButtonText("Kết nối thành công");

        await timeout(500);
        
        ServersConnectService?.connectTo(server);
    };

    return (
                <Flex vertical centered gap="large" className={s["right-side"]}>
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

                    {state.showUsernameField && (
                    <Input
                        fullWidth
                        label={$L('#AuthForm_FieldLabel_Username')}
                        placeholder="CarlJohnson07"
                        value={state.username.value}
                        onChange={state.username.set}
                        disabled={state.disabled}
                        onSubmit={handleSubmit}
                        //pattern={usernameRegexp}

                        decorator={state.renderUsernameDecorator()}
                    />
                    )}

                    {state.showTOTPField && (
                    <Input
                        inputRef={totpFieldRef}
                        autofocus
                        fullWidth
                        label={$L('#AuthForm_FieldLabel_TOTP')}
                        placeholder="112233"
                        value={state.totp}
                        onChange={state.setTOTP}
                        disabled={state.disabled}
                        onSubmit={handleSubmit}
                        description={$L('#AuthForm_FieldDescription_TOTP')}
                    />
                    )}

                    <SubmitControls state={state} testSubmit={handleSubmit} buttonText={buttonText}/>

                    
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
  
          {state.disabled ? <Indicator /> : null}
  
          <Button
            text={buttonText}
            theme={state.disabled ? 'default' : 'primary'}
            disabled={state.disabled || !state.canSubmit}
            onClick={testSubmit}
          />
        </Flex>
      );
    }
  
    if (state.isTOTP) {
      return (
        <Flex centered repell>
          <Button
            text={$L('#AuthForm_Back')}
            theme="transparent"
            disabled={state.disabled}
            onClick={state.switchToLogIn}
          />
  
          {state.disabled ? <Indicator /> : null}
  
          <Button
            text={buttonText}
            theme={state.disabled ? 'default' : 'primary'}
            disabled={state.disabled || !state.canSubmit}
            onClick={testSubmit}
          />
        </Flex>
      );
    }
  
    if (state.isExternal) {
      return (
        <Flex centered>
          <Button
            text={$L('#AuthForm_Back')}
            onClick={state.switchToLogIn}
          />
        </Flex>
      );
    }
  
    if (state.isRegistrationActivation) {
      return (
        <Flex centered repell>
          <Button
            text={$L('#AuthForm_Registration_ResendActivation')}
            disabled={state.disabled}
            onClick={state.resendActivationEmail}
          />
  
          {state.disabled ? <Indicator /> : null}
  
          <Button
            theme="primary"
            text={buttonText}
            disabled={state.disabled}
            onClick={testSubmit}
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
  