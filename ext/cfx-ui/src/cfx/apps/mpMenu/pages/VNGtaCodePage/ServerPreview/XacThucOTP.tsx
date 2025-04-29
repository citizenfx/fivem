import { Flex } from "cfx/ui/Layout/Flex/Flex";
import { observer } from "mobx-react-lite";
import { useState } from "react";
import './XacThucOTP.scss';
import { mpMenu } from "cfx/apps/mpMenu/mpMenu";
import { useLocation } from "react-router-dom";
import { IAuthFormState, totpFieldRef, useAuthFormState } from "cfx/common/parts/AuthForm/AuthFormState";
import { ToastContainer, toast } from 'react-toastify';
import 'react-toastify/dist/ReactToastify.css';
import { useNavigate } from "react-router-dom";
import { timeout } from "cfx/utils/async";

export const XacThucOTP = observer(function ServerPreview() {
    const location = useLocation();
    const username = location.state?.username;
    const navigate = useNavigate();
    const [isLoading, setIsLoading] = useState(false);
    const [otp, setOtp] = useState<string[]>(Array(6).fill('')); // Lưu trữ mã OTP

    const handleChange = (value: string, index: number) => {
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

    const handleKeyDown = (e: React.KeyboardEvent<HTMLInputElement>, index: number) => {
        if (e.key === 'Backspace' && !otp[index] && index > 0) {
            const prevInput = document.getElementById(`otp-input-${index - 1}`);
            prevInput?.focus();
        }
    };

    const handlePaste = (e: React.ClipboardEvent<HTMLInputElement>) => {
        e.preventDefault(); // Ngăn trình duyệt dán toàn bộ chuỗi vào một ô

        const pasteData = e.clipboardData.getData('text'); // Lấy dữ liệu từ clipboard
        if (!/^\d+$/.test(pasteData)) return; // Chỉ xử lý nếu dữ liệu là số

        const newOtp = [...otp];
        pasteData.split('').forEach((char, index) => {
            if (index < 6) {
                newOtp[index] = char;
            }
        });

        setOtp(newOtp);

        // Tự động focus vào ô cuối cùng đã điền
        const lastFilledIndex = Math.min(pasteData.length - 1, 5);
        const nextInput = document.getElementById(`otp-input-${lastFilledIndex}`);
        nextInput?.focus();
    };

    const handleSubmit = async () => {
        if (isLoading) return; // Ngăn gửi lại nếu đang trong quá trình xác minh
        try {
            const otpCode = otp.join('');
            if (otpCode.length === 6) {
                const response = await fetch('https://game.vngta.com:3979/api/verifyemail', {
                    method: 'POST',
                    headers: {
                        'Content-Type': 'application/json',
                        'Accept': 'application/json'
                    },
                    body: JSON.stringify({
                        username: username,
                        token: otpCode
                    }),
                });
                if (!response.ok) {
                    const errorData = await response.json();
                    throw new Error(errorData.message || 'Xác minh thất bại');
                }
                const data = await response.json();
                if (data.success) {
                    setIsLoading(true);
                    toast.success('Xác minh email thành công!');
                    await timeout(2000);
                    setIsLoading(false);
                    navigate(`/vngtalogin`);
                }else{
                    toast.error(data.message || 'Xác minh thất bại');
                    throw new Error(data.message || 'Xác minh thất bại');
                }
                
            } else {
                toast.error('Vui lòng nhập đầy đủ mã OTP!');
            }
        } catch (error) {
            
        }
    };

    return (
        <><ToastContainer
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
            <div className="otp-container">
                <div className="corner-right-bottom"></div>
                <h2>XÁC MINH EMAIL</h2>
                <p>Vui lòng nhập mã OTP được gửi đến email đăng ký của bạn</p>
                <Flex className="otp-inputs" gap="small">
                    {otp.map((digit, index) => (
                        <input
                            key={index}
                            id={`otp-input-${index}`}
                            type="text"
                            maxLength={1}
                            value={digit}
                            onChange={(e) => handleChange(e.target.value, index)}
                            onKeyDown={(e) => handleKeyDown(e, index)}
                            onPaste={handlePaste} // Thêm sự kiện onPaste
                            className="otp-input" />
                    ))}
                </Flex>
                <button className="otp-submit" onClick={handleSubmit}>
                    XÁC NHẬN
                </button>
                <p className="resend">
                    Bạn chưa nhận được mã? <span onClick={() => alert('Gửi lại mã OTP')}>GỬI LẠI</span>
                </p>
            </div></>
    );
});