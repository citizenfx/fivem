import { Component } from '@angular/core';
import { GameService } from '../game.service';
import { DiscourseAuthModalState, DiscourseService, DiscourseUser } from '../discourse.service';

@Component({
    selector: 'auth-modal',
    templateUrl: './auth-modal.component.html',
    styleUrls: ['./auth-modal.component.scss']
})
export class AuthModalComponent {
    modalVisible: boolean;

    accountInfoLoaded = false;

    login = true;
    register = false;
    completeRegister = false;
    completeAuth = false;
    authOnWeb = false;
    notification: string | null = null;
    btnActive = false;

    typedEmail = '';
    typedPassword = '';
    typedUserName = '';

    emailHint = 'Never shown to the public';
    usernameHint = 'Unique, No Spaces, Short';
    passwordHint = 'At least 8 characters';
    emailValid = false;
    usernameValid = false;
    passwordValid = false;

    streamerMode: boolean = true;
    showIgnoreButton: boolean = false;

    currentAccount: DiscourseUser | null = null;

    get isLoginScreen(): boolean {
        return this.accountInfoLoaded && !this.completeAuth && !this.completeRegister && !this.register && this.login;
    }

    get isRegisterScreen(): boolean {
        return this.accountInfoLoaded && !this.completeAuth && !this.completeRegister && !this.login && this.register;
    }

    get isAuthCompleteScreen(): boolean {
        return this.accountInfoLoaded && this.completeAuth;
    }

    get isRegistrationCompleteScreen(): boolean {
        return this.accountInfoLoaded && this.completeRegister;
    }

    constructor(
        private gameService: GameService,
        private discourseService: DiscourseService,
    ) {
		this.streamerMode = gameService.streamerMode;
        this.showIgnoreButton = discourseService.authModalState.getValue() === DiscourseAuthModalState.SHOWN;

        this.discourseService.authModalOpenChange.subscribe((authModalOpen) => this.modalVisible = authModalOpen);
        this.gameService.streamerModeChange.subscribe((streamerMode) => this.streamerMode = streamerMode);
        this.discourseService.authModalState.subscribe((state) => {
            if (state === DiscourseAuthModalState.SHOWN) {
                this.showIgnoreButton = true;
            }
        });

        this.discourseService.signinChange.subscribe((user) => this.currentAccount = user);
        this.discourseService.initialAuthComplete.subscribe((complete) => {
            this.accountInfoLoaded = complete;
        });
    }

    toggleModalVisiblity(): void {
        let analyticsName: string;

        if (this.isLoginScreen) {
            analyticsName = 'LoginScreen';
        } else if (this.isRegisterScreen) {
            analyticsName = 'RegistrationScreen';
        }

        this.discourseService.closeAuthModal(analyticsName);
    }

    dontAskAgain() {
        this.discourseService.closeAuthModalAndIgnore();
    }

    setLoginView(): void {
        this.login = true;
        this.register = false;
        this.completeRegister = false;
        this.completeAuth = false;
        this.typedEmail = '';
        this.typedPassword = '';
        this.typedUserName = '';
        this.emailValid = false;
        this.usernameValid = false;
        this.passwordValid = false;
    }

    setRegisterView(): void {
        this.register = true;
        this.login = false;
        this.completeRegister = false;
        this.completeAuth = false;
        this.typedEmail = '';
        this.typedPassword = '';
        this.typedUserName = '';
    }

    validateEmail(email: string) {
        const re = /^[a-zA-Z0-9!#\$%&'*+\/=?\^_`{|}~\-]+(?:\.[a-zA-Z0-9!#\$%&'\*+\/=?\^_`{|}~\-]+)*@(?:[a-zA-Z0-9](?:[a-zA-Z0-9\-]*[a-zA-Z0-9])?\.)+[a-zA-Z0-9](?:[a-zA-Z0-9\-]*[a-zA-Z0-9])?$/;
        return re.test(email);
    }

    async onEmailChange() {
        if (this.typedEmail === '') {
            this.emailValid = false;
            this.emailHint = 'Never shown to the public';
        } else if (this.validateEmail(this.typedEmail)) {
            let emailAvailable = await this.discourseService.checkValidEmail(this.typedEmail);
            if (emailAvailable.success) {
                this.emailValid = true;
                this.emailHint = 'Valid Email Address';
            } else {
                this.emailValid = false;
                this.emailHint = emailAvailable.errors[0];
            }
        } else {
            this.emailValid = false;
            this.emailHint = 'Please enter a valid email';
        }
    }

    async onUsernameChange() {
        if (this.typedUserName === '') {
            this.usernameValid = false;
            this.usernameHint = 'Unique, No Spaces, Short';
        } else if (this.typedUserName.length < 3) {
            this.usernameValid = false;
            this.usernameHint = 'Your username is too short';
        } else {
            const usernameAvailable = await this.discourseService.checkValidUsername(this.typedUserName);
            if (usernameAvailable.available === false) {
                this.usernameValid = false;
                this.usernameHint = `Not available. Try ${usernameAvailable.suggestion}`;
            } else {
                this.usernameValid = true;
                this.usernameHint = `Your username is available`;
            }
        }
    }

    onPasswordChange(): void {
        if (this.typedPassword === '') {
            this.passwordValid = false;
        } else {
            this.typedPassword.length >= 8 ? this.passwordValid = true : this.passwordValid = false;
        }
    }

    async oneClickDiscourseLogin() {
        this.preventBtnSpam(async () => {
            const url = await this.discourseService.generateAuthURL();
            this.gameService.openUrl(url);
            this.authOnWeb = true;
            this.completeAuth = true;
        }, 3000);
    }

    async traditionalDiscourseLogin() {
        this.preventBtnSpam(async () => {
            try {
                const user = await this.discourseService.login(this.typedEmail, this.typedPassword);
                if (user.error) {
                    this.setNotification(user.error, 7000);
                    return;
                }
                this.completeRegister = false;
                this.completeAuth = true;
            } catch {
                this.setNotification('Error signing In - Try again later', 7000);
            }
        }, 2000);
    }

    forgotDiscoursePassword(): void {
        this.preventBtnSpam(async () => {
            if (this.typedEmail === '' || !this.validateEmail(this.typedEmail)) {
                this.setNotification('Please enter a valid email to reset password', 7000);
                return;
            }
            try {
                await this.discourseService.resetPassword(this.typedEmail);
                this.setNotification('Password reset email sent', 7000);
            } catch {
                this.setNotification('Failed to send reset email - Try reset on forum.cfx.re', 7000);
            }
        }, 2000);
    }

    async registerDiscourseAccount() {
        this.preventBtnSpam(async () => {
            const newUserInfo = {
                'email': this.typedEmail,
                'username': this.typedUserName,
                'password': this.typedPassword
            };

            const newUser = await this.discourseService.registerNewUser(newUserInfo);

            if (newUser.success) {
                this.completeRegister = true;
            } else if (newUser.success === false) {
                const errs = newUser.errors;
                const allErrsArray = [];
                for (const err in errs) {
                    errs[err].forEach(e => {
                        if (typeof e === 'string') {
                            allErrsArray.push(e);
                        }
                    });
                }

                const notificationMsg = allErrsArray[0] === undefined ? 'Account Creation Failed. Try Again Later' : allErrsArray.join(' ');

                this.setNotification(notificationMsg, 5000);
            } else {
                this.setNotification('Account Creation Failed. Try Again Later', 5000);
            }
        }, 2000);
    }

    async resendDiscourseActivationEmail() {
        this.preventBtnSpam(async () => {
            try {
                await this.discourseService.resendActivationEmail(this.typedUserName);
                this.setNotification('Activation Email Sent', 5000);
            } catch {
                this.setNotification('Failed to resend Activation Email Sent', 5000);
            }
        }, 2000);
    }

    setNotification(notiMsg: string, notiDisplayTime: number): void {
        this.notification = notiMsg;
        setTimeout(() => {
            this.notification = null;
        }, notiDisplayTime);
    }

    preventBtnSpam(fn, time: number): void {
        if (!this.btnActive) {
            this.btnActive = true;
            fn();
            setTimeout(() => {
                this.btnActive = false;
            }, time);
        } else {
            return;
        }
    }
}
