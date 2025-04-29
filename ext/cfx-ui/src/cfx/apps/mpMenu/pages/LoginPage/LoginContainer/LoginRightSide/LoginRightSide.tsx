// export {}
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

import { ToastContainer, toast } from 'react-toastify';
import 'react-toastify/dist/ReactToastify.css';

export const LoginRightSide = observer(function LoginRightSide() {

  const [subtitle, setSubtitle] = React.useState("Thành phố của bạn, luật lệ của bạn");

  const AccountService = useAccountService();
  const state = useAuthFormState();
  const ServersConnectService = useServiceOptional(IServersConnectService);

  const server = useLastHistoryServer();
  const te: any = $L('#AuthForm_LogIn_Submit');
  const [buttonText, setButtonText] = React.useState(te);
  const [isRegistering, setIsRegistering] = React.useState(false);

  // Thêm state cho chức năng lưu đăng nhập
  const [rememberLogin, setRememberLogin] = React.useState(false);
  
  // Các state cho form đăng ký
  const [firstName, setFirstName] = React.useState('');
  const [lastName, setLastName] = React.useState('');
  const [birthDate, setBirthDate] = React.useState('');
  const [gender, setGender] = React.useState('male'); // Mặc định là nam
  
  const navigate = useNavigate();

  // Kiểm tra nếu có thông tin đăng nhập đã lưu khi component mount
  React.useEffect(() => {
    const savedUsername = localStorage.getItem('vngta_username');
    const savedPassword = localStorage.getItem('vngta_password');
    
    if (savedUsername && savedPassword) {
      state.setusername2(savedUsername);
      state.setPassword(savedPassword);
      
      // Tự động đặt trạng thái lưu đăng nhập
      setRememberLogin(true);
    }
  }, []);

  const entityMap: Record<string, string> = {
    '&': '&amp;',
    '<': '&lt;',
    '>': '&gt;',
    '"': '&quot;',
    "'": '&#39;',
    '/': '&#x2F;',
    '=': '&#x3D;'
  };
  
  // Định nghĩa hàm escapeHtml với kiểu dữ liệu
  function escapeHtml(string: string): string {
    return String(string).replace(/[&<>"'=/]/g, function(s) {
      return entityMap[s];
    });
  }

  const handleSubmit = async () => {
    try {
      // Kiểm tra xem email và password có giá trị không
      if (!state.username2 || !state.password) {
        return;
      }

      state.submitPending = true;
      setButtonText("Đang đăng nhập...");

      const sanitizedUsername = escapeHtml(state.username2.trim().toLowerCase());

      console.log("Đang gửi request đăng nhập với User:", sanitizedUsername);

      // Gửi request đến server để kiểm tra đăng nhập
      const response = await fetch('https://game.vngta.com:3979/api/login', {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
          'Accept': 'application/json'
        },
        body: JSON.stringify({
          username: sanitizedUsername,
          password: state.password,
          discord: mpMenu.discordIdZ.value,
        }),
      });

      console.log("Nhận response với status:", response.status);
      if (!response.ok) {
        const errorData = await response.json();
        throw new Error(errorData.message || 'Đăng nhập thất bại');
      }
      const data = await response.json();

      // Kiểm tra kết quả từ server
      if (data.success) {

        mpMenu.setPlayerCount(data.MemBerS);

        // Nếu người dùng chọn lưu đăng nhập, lưu thông tin vào localStorage

        if (rememberLogin) {

          localStorage.setItem('vngta_username', sanitizedUsername);

          localStorage.setItem('vngta_password', state.password);

        } else {

          // Nếu không chọn lưu đăng nhập, xóa thông tin đã lưu (nếu có)

          localStorage.removeItem('vngta_username');

          localStorage.removeItem('vngta_password');

        }

        // Gọi NUI cho FiveM
        mpMenu.invokeNative('setIndentify', sanitizedUsername);
        mpMenu.invokeNative('setcitizenid', data.citizenid);
        
        // Delay nhẹ để hiển thị thông báo thành công (tùy chọn)
        await timeout(500);
        if (data.active) {
          navigate(`/vngtacheckCode`, { 
            state: { 
              username: sanitizedUsername 
            } 
          });
        }else{
          navigate(`/vngtalogin`);
        }

        // Chuyển hướng đến trang vngtalogin
        // navigate(`/vngtacheckCode`);
      } else {
        throw new Error(data.message || 'Đăng nhập không thành công');
      }
    } catch (error) {
      toast.error(error.message);
      state.submitPending = false;
      setButtonText(te);
      
      // Hiển thị thông báo lỗi
    }
  };

  const handleRegister = async () => {
    try {
      // Kiểm tra xem tất cả các trường đăng ký có giá trị không
      if (!state.email.value || !state.password || !state.username2 || !firstName || !lastName || !birthDate) {
        return;
      }

      if (!state.email.value.match(/^[a-zA-Z0-9._-]+@gmail\.com$/i)) {
        toast.error("Email không hợp lệ");
        state.submitPending = false;
        return;
      }

      state.submitPending = true;
      setButtonText("Đang đăng ký...");

       // Làm sạch dữ liệu trước khi gửi đi
      const sanitizedEmail = escapeHtml(state.email.value.trim().toLowerCase());
      const sanitizedUsername = escapeHtml(state.username2.trim().toLowerCase());
      const sanitizedFirstName = escapeHtml(firstName.trim());
      const sanitizedLastName = escapeHtml(lastName.trim());
      // Gửi request đến server để đăng ký
      const response = await fetch('https://game.vngta.com:3979/api/register', {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
          'Accept': 'application/json'
        },
        body: JSON.stringify({
          email: sanitizedEmail,
          discordId: mpMenu.discordIdZ.value,
          password: state.password,
          username: sanitizedUsername,
          firstName: sanitizedFirstName,
          lastName: sanitizedLastName,
          birthDate: birthDate,
          gender: gender
        }),
      });

      console.log("Nhận response với status:", response.status);
      
      if (!response.ok) {
        const errorData = await response.json();
        throw new Error(errorData.message || 'Đăng ký thất bại');
      }
      
      const data = await response.json();

      // Kiểm tra kết quả từ server
      if (data.success) {
        // Đăng ký thành công
        console.log('Đăng ký thành công');
        
        // Chuyển về trạng thái đăng nhập
        setIsRegistering(false);
        toast.success("Đăng ký thành công! Vui lòng đăng nhập");
        
        // Reset các trường đăng ký
        state.setusername2('');
        setFirstName('');
        setLastName('');
        setBirthDate('');
        setGender('male');
        setButtonText(te);
      } else {
        // Đăng ký thất bại
        throw new Error(data.message || 'Đăng ký không thành công');
      }
    } catch (error) {
      toast.error(error.message);
      state.submitPending = false;
      setButtonText('Đăng ký');
      
      // Hiển thị thông báo lỗi
    } finally {
      state.submitPending = false;
    }
  };

  const toggleRegisterMode = () => {
    const newIsRegistering = !isRegistering;
    setIsRegistering(newIsRegistering);
    
    // Thay đổi subtitle khi chuyển chế độ
    setSubtitle(
      newIsRegistering 
        ? "ĐĂNG KÝ TÀI KHOẢN" 
        : "Thành phố của bạn, luật lệ của bạn!"
    );

    setButtonText(isRegistering ? te : "Đăng ký");
    
    // Reset các trường đăng ký khi chuyển đổi chế độ
    if (!isRegistering) {
      // Khi chuyển từ đăng nhập sang đăng ký, reset các trường
      state.setusername2('');
      setFirstName('');
      setLastName('');
      setBirthDate('');
      setGender('male');
      state.setPassword('');
    }
  };
  
  // Kiểm tra xem form đăng ký có hợp lệ không
  const isRegisterFormValid = () => {
    return state.email.value && 
           state.password && 
           state.username2 && 
           firstName && 
           lastName && 
           birthDate;
  };

  // Tạo style cho radio button
  const radioStyle = {
    marginRight: '10px',
    cursor: 'pointer'
  };
  
  const radioLabelStyle = {
    cursor: 'pointer',
    marginRight: '20px'
  };

  // Handler cho checkbox lưu đăng nhập
  const handleRememberLoginChange = () => {
    if (rememberLogin) {
      // Nếu đang được check và sắp bị uncheck, xóa dữ liệu localStorage
      localStorage.removeItem('vngta_username');
      localStorage.removeItem('vngta_password');
    }
    setRememberLogin(!rememberLogin);
  };

  return (
    <Flex vertical centered gap="large" className={s["right-side"]}>
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
      <Flex centered className={s["server-information"]}>
        <img className={s.logo} src="https://raw.githubusercontent.com/TruongVoKyDeep/vnpd/refs/heads/main/luutru/omegaz_cri.png" />
        <div>
          <Text size="xlarge" className={s.title} asDiv>VNGTA</Text>
          <Text size="small" className={s.title2} asDiv>{subtitle}</Text>
        </div>
      </Flex>
      
      <Input
        fullWidth
        label="Tên người dùng"
        placeholder="Nhập tên người dùng của bạn"
        value={state.username2}
        onChange={state.setusername2}
        disabled={state.disabled}
      />

      {state.showPasswordField && (
        <Input
          fullWidth
          type={isRegistering ? 'text' : 'password'}
          label={$L('#AuthForm_FieldLabel_Password')}
          placeholder="Nhập mật khẩu của bạn"
          value={state.password}
          onChange={state.setPassword}
          disabled={state.disabled}
          onSubmit={isRegistering ? handleRegister : handleSubmit}
          // decorator={isRegistering ? null : state.renderPasswordDecorator()}
        />
      )}
      {/* Thêm checkbox Lưu đăng nhập khi ở chế độ đăng nhập */}
      {!isRegistering && (
        <Flex fullWidth>
          <div style={{ display: 'flex', alignItems: 'center', width: '100%' }}>
            <div style={{ display: 'flex', alignItems: 'center' }}>
              <input 
                type="checkbox" 
                id="rememberLogin" 
                checked={rememberLogin} 
                onChange={handleRememberLoginChange}
                style={{ ...radioStyle, marginTop: '0' }}
                disabled={state.disabled}
              />
              <label htmlFor="rememberLogin" style={radioLabelStyle}>Lưu đăng nhập</label>
            </div>
            <div 
              style={{ marginLeft: 'auto', cursor: 'pointer' }}
              onClick={() => navigate(`/vngtaresetpass`)}
            >
              <label className={s.quenmatkhau}>
                Quên mật khẩu?
              </label>
            </div>
          </div>
        </Flex>
      )}

      {/* Các trường thông tin bổ sung cho đăng ký */}
      {isRegistering && (
        <>
          {state.showEmailField && (
            <Input
              autofocus
              fullWidth
              label={$L('#AuthForm_FieldLabel_Email')}
              placeholder="vngta@gmail.com"
              value={state.email.value}
              onChange={state.email.set}
              disabled={state.disabled}
              onSubmit={isRegistering ? handleRegister : handleSubmit}
              decorator={state.renderEmailDecorator()}
            />
          )}
          
          <Input
            fullWidth
            label="Họ"
            placeholder="Nhập họ của bạn"
            value={lastName}
            onChange={setLastName}
            disabled={state.disabled}
          />
          
          <Input
            fullWidth
            label="Tên"
            placeholder="Nhập tên của bạn"
            value={firstName}
            onChange={setFirstName}
            disabled={state.disabled}
          />
          
          <Input
            fullWidth
            label="Ngày sinh"
            placeholder="YYYY-MM-DD"
            value={birthDate}
            onChange={setBirthDate}
            disabled={state.disabled}
            type="date"
            min="1940-01-01"  // Giới hạn năm tối thiểu
            max="2025-12-31"  // Giới hạn năm tối đa
          />
          
          <Flex vertical fullWidth>
            <Text size="normal">Giới tính</Text>
            <Flex gap="small" fullWidth className={s.dateInputContainer}>
              <div>
                <input 
                  type="radio" 
                  id="male" 
                  name="gender" 
                  value="male" 
                  checked={gender === 'male'} 
                  onChange={() => setGender('male')}
                  style={radioStyle}
                  disabled={state.disabled}
                />
                <label htmlFor="male" style={radioLabelStyle}>Nam</label>
              </div>
              
              <div>
                <input 
                  type="radio" 
                  id="female" 
                  name="gender" 
                  value="female" 
                  checked={gender === 'female'} 
                  onChange={() => setGender('female')}
                  style={radioStyle}
                  disabled={state.disabled}
                />
                <label htmlFor="female" style={radioLabelStyle}>Nữ</label>
              </div>
            </Flex>
          </Flex>
        </>
      )}

      {/* Các nút điều khiển */}
      <Flex centered repell>
        <Button
          text={isRegistering ? "Quay lại đăng nhập" : "Đăng Ký"}
          theme="transparent"
          disabled={state.disabled}
          onClick={toggleRegisterMode}
        />

        {state.disabled ? <Indicator /> : null}

        <Button
          text={buttonText}
          theme={state.disabled ? 'default' : 'primary'}
          disabled={
            state.disabled || 
            (isRegistering 
              ? !isRegisterFormValid() 
              : !(state.username2 && state.password)
            )
          }
          // disabled={state.disabled || (!isRegistering && !state.canSubmit) || (isRegistering && !isRegisterFormValid())}
          onClick={isRegistering ? handleRegister : handleSubmit}
          icon={state.submitPending ? <Indicator /> : null}
        />
      </Flex>
    </Flex>
  );
});