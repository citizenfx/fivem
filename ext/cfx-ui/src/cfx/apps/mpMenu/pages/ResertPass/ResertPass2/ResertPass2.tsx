import { Flex } from "cfx/ui/Layout/Flex/Flex";
import { observer } from "mobx-react-lite";
import { useState } from "react";
import './ResertPass2.scss';
import { mpMenu } from "cfx/apps/mpMenu/mpMenu";
import { useLocation } from "react-router-dom";
import { IAuthFormState, totpFieldRef, useAuthFormState } from "cfx/common/parts/AuthForm/AuthFormState";
import { ToastContainer, toast } from 'react-toastify';
import 'react-toastify/dist/ReactToastify.css';
import { useNavigate } from "react-router-dom";
import { timeout } from "cfx/utils/async";

export const ResertPass2 = observer(function ResertPass() {
    const navigate = useNavigate();
    const [isLoading, setIsLoading] = useState(false);
    const [email, setGmail] = useState('');
    const [step, setStep] = useState(1); // 1: Nhập Gmail, 2: Nhập OTP, 3: Đặt mật khẩu mới
    const [otp, setOtp] = useState<string[]>(Array(6).fill(''));
    const [newPassword, setNewPassword] = useState('');
    const [confirmPassword, setConfirmPassword] = useState('');
    
    // Xử lý nhập Gmail
    const handleChangeGmail = (e: React.ChangeEvent<HTMLInputElement>) => {
        setGmail(e.target.value);
    };

    // Xử lý nhập mã OTP
    const handleOtpChange = (value: string, index: number) => {
        if (!/^\d*$/.test(value)) return; // Chỉ cho phép nhập số

        const newOtp = [...otp];
        newOtp[index] = value;
        setOtp(newOtp);

        // Tự động chuyển sang ô tiếp theo
        if (value && index < 5) {
            const nextInput = document.getElementById(`otp-input-${index + 1}`);
            nextInput?.focus();
        }
    };

    // Xử lý khi nhấn backspace trong OTP
    const handleKeyDown = (e: React.KeyboardEvent<HTMLInputElement>, index: number) => {
        if (e.key === 'Backspace' && !otp[index] && index > 0) {
            const prevInput = document.getElementById(`otp-input-${index - 1}`);
            prevInput?.focus();
        }
    };

    // Xử lý khi paste OTP
    const handlePaste = (e: React.ClipboardEvent<HTMLInputElement>) => {
        e.preventDefault();
        const pasteData = e.clipboardData.getData('text');
        if (!/^\d+$/.test(pasteData)) return;

        const newOtp = [...otp];
        pasteData.split('').forEach((char, index) => {
            if (index < 6) {
                newOtp[index] = char;
            }
        });

        setOtp(newOtp);

        const lastFilledIndex = Math.min(pasteData.length - 1, 5);
        const nextInput = document.getElementById(`otp-input-${lastFilledIndex}`);
        nextInput?.focus();
    };

    // Xử lý nhập mật khẩu mới
    const handleNewPasswordChange = (e: React.ChangeEvent<HTMLInputElement>) => {
        setNewPassword(e.target.value);
    };

    // Xử lý nhập xác nhận mật khẩu
    const handleConfirmPasswordChange = (e: React.ChangeEvent<HTMLInputElement>) => {
        setConfirmPassword(e.target.value);
    };

    // Gửi yêu cầu đặt lại mật khẩu
    const handleSubmitEmail = async () => {
        if (isLoading) return;
        
        // Kiểm tra gmail hợp lệ
        const gmailRegex = /^[a-zA-Z0-9._-]+@gmail\.com$/i;
        if (!gmailRegex.test(email)) {
            toast.error('Vui lòng nhập địa chỉ Gmail hợp lệ!');
            return;
        }
        
        setIsLoading(true);
        try {
            // Gửi request đến server để xác minh gmail
            const response = await fetch('https://game.vngta.com:3979/api/requestresetpassword', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json',
                    'Accept': 'application/json'
                },
                body: JSON.stringify({
                    gmail: email
                }),
            });

            if (!response.ok) {
                const errorData = await response.json();
                throw new Error(errorData.message || 'Yêu cầu thất bại');
            }

            const data = await response.json();
            if (data.success) {
                toast.success('Mã xác nhận đã được gửi đến gmail của bạn!');
                await timeout(1000);
                // Chuyển sang bước nhập OTP
                setStep(2);
            } else {
                throw new Error(data.message || 'Yêu cầu thất bại');
            }
        } catch (error) {
            toast.error(error.message || 'Đã xảy ra lỗi. Vui lòng thử lại sau!');
        } finally {
            setIsLoading(false);
        }
    };

    // Xác thực mã OTP
    const handleVerifyOtp = async () => {
        if (isLoading) return;
        
        const otpCode = otp.join('');
        if (otpCode.length !== 6) {
            toast.error('Vui lòng nhập đầy đủ mã xác nhận!');
            return;
        }
        
        setIsLoading(true);
        try {
            // Gửi request để xác thực OTP
            const response = await fetch('https://game.vngta.com:3979/api/verifyresetcode', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json',
                    'Accept': 'application/json'
                },
                body: JSON.stringify({
                    gmail: email,
                    code: otpCode
                }),
            });

            if (!response.ok) {
                const errorData = await response.json();
                throw new Error(errorData.message || 'Xác thực thất bại');
            }

            const data = await response.json();
            if (data.success) {
                toast.success('Mã xác nhận hợp lệ!');
                await timeout(1000);
                // Chuyển sang bước đặt mật khẩu mới
                setStep(3);
            } else {
                throw new Error(data.message || 'Mã xác nhận không hợp lệ');
            }
        } catch (error) {
            toast.error(error.message || 'Đã xảy ra lỗi. Vui lòng thử lại sau!');
        } finally {
            setIsLoading(false);
        }
    };

    // Đặt mật khẩu mới
    const handleResetPassword = async () => {
        if (isLoading) return;
        
        // Kiểm tra mật khẩu
        if (newPassword.length < 6) {
            toast.error('Mật khẩu phải có ít nhất 6 ký tự!');
            return;
        }
        
        if (newPassword !== confirmPassword) {
            toast.error('Mật khẩu xác nhận không khớp!');
            return;
        }
        
        setIsLoading(true);
        try {
            // Gửi request đến server để đặt lại mật khẩu
            const response = await fetch('https://game.vngta.com:3979/api/resetpassword', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json',
                    'Accept': 'application/json'
                },
                body: JSON.stringify({
                    gmail: email,
                    newPassword: newPassword
                }),
            });

            if (!response.ok) {
                const errorData = await response.json();
                throw new Error(errorData.message || 'Đặt lại mật khẩu thất bại');
            }

            const data = await response.json();
            if (data.success) {
                toast.success('Đặt lại mật khẩu thành công!');
                toast.success('Bạn sẽ được chuyển về trang đăng nhập sau 3 giây!');
                await timeout(3000);
                navigate('/');
            } else {
                throw new Error(data.message || 'Đặt lại mật khẩu thất bại');
            }
        } catch (error) {
            toast.error(error.message || 'Đã xảy ra lỗi. Vui lòng thử lại sau!');
        } finally {
            setIsLoading(false);
        }
    };

    // Quay lại bước trước
    const handleBack = () => {
        if (step === 3) {
            setStep(1);
        }else if (step > 1) {
            setStep(step - 1);
        } else {
            navigate('/');
        }
    };

    // Quay lại trang đăng nhập
    const handleBacktoLogin = async () => {
        navigate(`/`);
    };

    return (
        <>
            <ToastContainer
                position="top-right"
                autoClose={3000}
                hideProgressBar={false}
                newestOnTop={false}
                closeOnClick
                rtl={false}
                pauseOnFocusLoss
                draggable
                pauseOnHover 
            />
            
            {/* Bước 1: Nhập Gmail */}
            {step === 1 && (
                <div className="email-verification-container">
                    <div className="corner-right-bottom"></div>
                    <h2>LẤY LẠI MẬT KHẨU</h2>
                    <p>Mã xác nhận sẽ được gửi đến gmail của bạn</p>
                    <p className="note">Kiểm tra cả hộp thư rác nếu không thấy</p>
                    
                    <div className="input-container">
                        <label className="input-label">NHẬP GMAIL</label>
                        <input
                            type="email"
                            value={email}
                            onChange={handleChangeGmail}
                            className="email-input"
                            placeholder="Nhập địa chỉ gmail của bạn"
                        />
                    </div>
                    <button 
                        className="submit-button" 
                        onClick={handleSubmitEmail}
                        disabled={isLoading}
                    >
                        TIẾP TỤC {isLoading ? "..." : ""}
                    </button>
                    
                    <p className="backtologin">
                        <span onClick={handleBacktoLogin}>QUAY LẠI ĐĂNG NHẬP</span>
                    </p>
                </div>
            )}
            
            {/* Bước 2: Nhập mã OTP */}
            {step === 2 && (
                <div className="email-verification-container">
                    <div className="corner-right-bottom"></div>
                    <h2>XÁC MINH MÃ</h2>
                    <p>Vui lòng nhập mã xác nhận đã được gửi đến gmail của bạn</p>
                    <p className="note">Mã xác nhận gồm 6 chữ số</p>
                    
                    <Flex className="otp-inputs" gap="small">
                        {otp.map((digit, index) => (
                            <input
                                key={index}
                                id={`otp-input-${index}`}
                                type="text"
                                maxLength={1}
                                value={digit}
                                onChange={(e) => handleOtpChange(e.target.value, index)}
                                onKeyDown={(e) => handleKeyDown(e, index)}
                                onPaste={handlePaste}
                                className="otp-input" />
                        ))}
                    </Flex>
                    
                    <button 
                        className="submit-button" 
                        onClick={handleVerifyOtp}
                        disabled={isLoading || otp.join('').length !== 6}
                    >
                        TIẾP TỤC {isLoading ? "..." : ""}
                    </button>
                    
                    <p className="backtologin">
                        <span onClick={handleBack}>QUAY LẠI</span>
                    </p>
                </div>
            )}
            
            {/* Bước 3: Đặt mật khẩu mới */}
            {step === 3 && (
                <div className="email-verification-container">
                    <div className="corner-right-bottom"></div>
                    <h2>ĐẶT MẬT KHẨU MỚI</h2>
                    <p>Vui lòng nhập mật khẩu mới cho tài khoản của bạn</p>
                    <p className="note">Mật khẩu phải có ít nhất 6 ký tự</p>
                    
                    <div className="input-container">
                        <label className="input-label">MẬT KHẨU MỚI</label>
                        <input
                            type="password"
                            value={newPassword}
                            onChange={handleNewPasswordChange}
                            className="email-input"
                            placeholder="Nhập mật khẩu mới"
                        />
                    </div>
                    
                    <div className="input-container">
                        <label className="input-label">XÁC NHẬN MẬT KHẨU</label>
                        <input
                            type="password"
                            value={confirmPassword}
                            onChange={handleConfirmPasswordChange}
                            className="email-input"
                            placeholder="Nhập lại mật khẩu mới"
                        />
                    </div>
                    
                    <button 
                        className="submit-button" 
                        onClick={handleResetPassword}
                        disabled={isLoading || !newPassword || !confirmPassword}
                    >
                        XÁC NHẬN {isLoading ? "..." : ""}
                    </button>
                    
                    <p className="backtologin">
                        <span onClick={handleBack}>QUAY LẠI</span>
                    </p>
                </div>
            )}
        </>
    );
});