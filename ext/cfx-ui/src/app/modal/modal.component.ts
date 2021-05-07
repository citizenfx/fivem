import { Component, OnInit, Input } from '@angular/core';
import { GameService } from '../game.service';
import { DiscourseService, DiscourseUser } from '../discourse.service';

@Component({
    selector: 'app-modal',
    templateUrl: './modal.component.html',
    styleUrls: ['./modal.component.scss']
})
export class ModalComponent implements OnInit {
    modalVisible = false;
    modalVisibleDelayComplete: boolean = false;
    initialLoad: boolean = true;
    login: boolean = false;
    register: boolean = false;
    completeRegister: boolean = false;
    completeAuth: boolean = false;
    authOnWeb: boolean = false;
    notification: string = null;
    btnActive: boolean = false;

    typedEmail: string = '';
    typedPassword: string = '';
    typedUserName: string = '';

    emailHint: string = 'Never shown to the public';
    usernameHint: string = 'Unique, No Spaces, Short';
    passwordHint: string = 'At least 8 characters';
    emailValid: boolean = false;
    usernameValid: boolean = false;
    passwordValid: boolean = false;

    @Input()
    streamerMode: boolean;

    @Input()
    currentAccount: DiscourseUser;

    constructor(
        private gameService: GameService,
        private discourseService: DiscourseService,
    ) {
        this.discourseService.signinChange.subscribe(user => {
            //prevents already signed in users from seeing modal
            //shows modal auth complete page if user signs in within fivem
            if (user) {
                if (this.initialLoad) {
                    this.modalVisible = false;
                } else {
                    this.modalVisible = true;
                }
            } else {
                this.modalVisible = true;
            }
        });
    }

    ngOnInit(): void {
        setTimeout(() => {
            this.modalVisibleDelayComplete = true;
            this.initialLoad = false;
        }, 2000);
    }

    toggleModalVisiblity(): void {
        this.modalVisible = !this.modalVisible;
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
        const re = /^(([^<>()[\]\\.,;:\s@\"]+(\.[^<>()[\]\\.,;:\s@\"]+)*)|(\".+\"))@((\[[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\])|(([a-zA-Z\-0-9]+\.)+[a-zA-Z]{2,}))$/;
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
                this.setNotification('Please enter a valid email to reset', 7000);
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
