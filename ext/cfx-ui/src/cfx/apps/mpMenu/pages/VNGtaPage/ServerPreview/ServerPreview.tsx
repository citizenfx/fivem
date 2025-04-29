import { Flex } from "cfx/ui/Layout/Flex/Flex";
import { observer } from "mobx-react-lite";
import { Navigation, Pagination, EffectCoverflow } from 'swiper';
import { Swiper, SwiperSlide, useSwiper, useSwiperSlide } from 'swiper/react';
import './ServerPreview.scss';
import { Text } from "cfx/ui/Text/Text";
import { Button, ButtonProps } from "cfx/ui/Button/Button";

import { mpMenu } from "cfx/apps/mpMenu/mpMenu";

// Import Swiper styles
import 'swiper/css';
import 'swiper/css/effect-coverflow';
import 'swiper/css/pagination';
import 'swiper/css/navigation';
import { PreviewBackgroundResponse } from "cfx/apps/mpMenu/services/serverPreview/serverPreview.service";
import { useState, useEffect  } from "react";
import { Icons } from "cfx/ui/Icons";
import { useNavigate } from "react-router-dom";
import { useService, useServiceOptional } from "cfx/base/servicesContainer";
import { IServersService } from "cfx/common/services/servers/servers.service";
import { IServersConnectService } from "cfx/common/services/servers/serversConnect.service";

// Tạo mảng đối tượng cố định với thêm trường enabled
const serverList = [
  {
    id: 1,
    name: "Dev Server",
    ip: "game.vngta.com:30120",
    image: "https://www.psu.com/wp/wp-content/uploads/2020/09/Grand-Theft-Auto-V-PS4-Wallpapers-09.jpg",
    enabled: false
  },
  {
    id: 2,
    name: "VNGTA Roleplay",
    ip: "game.vngta.com:30120",
    image: "https://images-wixmp-ed30a86b8c4ca887773594c2.wixmp.com/f/8d3e93c2-7eac-41fa-93af-8ba56fe6a863/df4r2vb-4063d5d6-196b-4027-81cd-2bd0dba50598.png/v1/fill/w_1280,h_720,q_80,strp/gta_v_roleplay_bennys_photo_by_amarmaruuuf_df4r2vb-fullview.jpg?token=eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzI1NiJ9.eyJzdWIiOiJ1cm46YXBwOjdlMGQxODg5ODIyNjQzNzNhNWYwZDQxNWVhMGQyNmUwIiwiaXNzIjoidXJuOmFwcDo3ZTBkMTg4OTgyMjY0MzczYTVmMGQ0MTVlYTBkMjZlMCIsIm9iaiI6W1t7ImhlaWdodCI6Ijw9NzIwIiwicGF0aCI6IlwvZlwvOGQzZTkzYzItN2VhYy00MWZhLTkzYWYtOGJhNTZmZTZhODYzXC9kZjRyMnZiLTQwNjNkNWQ2LTE5NmItNDAyNy04MWNkLTJiZDBkYmE1MDU5OC5wbmciLCJ3aWR0aCI6Ijw9MTI4MCJ9XV0sImF1ZCI6WyJ1cm46c2VydmljZTppbWFnZS5vcGVyYXRpb25zIl19.6Yc0d60Ulp1mVtcP3OJRMgldkfaLj7mIiU2YXIs-F-c",
    enabled: true
  },
  {
    id: 3,
    name: "Server Cho Thuê",
    ip: "game.vngta.com:30120",
    image: "https://i.ytimg.com/vi/RTdhEmCeQng/hq720.jpg?sqp=-oaymwEhCK4FEIIDSFryq4qpAxMIARUAAAAAGAElAADIQj0AgKJD&rs=AOn4CLDPsletE_PT4AGSAO0iNqrSKxSXUQ",
    enabled: false
  }
];

export const ServerPreview = observer(function ServerPreview({ previewBg }: { previewBg: PreviewBackgroundResponse[] | null }) {
    const [activeIndex, setActiveIndex] = useState(0);
    const navigate = useNavigate();
    const ServersService = useService(IServersService);
    const ServersConnectService = useServiceOptional(IServersConnectService);

    const handleSubmit = (serverIp: string, enabled: boolean) => {
      if (enabled) {
        ServersConnectService?.connectTo(serverIp);
      }
    }
    
    return (
        <Flex className="server-preview" centered vertical gap="xlarge">
            <Flex centered>
              <img className="logo" src="https://raw.githubusercontent.com/TruongVoKyDeep/vnpd/refs/heads/main/luutru/omegaz_cri.png" />
              <div>
                <Text size="xlarge" className="title" asDiv={true}>VNGTA</Text>
                <Text size="small" className="title2" asDiv={true}>Có những khoảnh khắc hài hước</Text>
              </div>
            </Flex>
            <Swiper
                effect={'coverflow'}
                centeredSlides={true}
                slidesPerView={'auto'}
                coverflowEffect={{
                  rotate: 0,
                  stretch: 0,
                  depth: 100,
                  modifier: 1.5
                }}
                initialSlide={1}
                spaceBetween={-100}
                pagination={{ el: '.swiper-pagination', clickable: true }}
                navigation={{
                  nextEl: '.swiper-button-next',
                  prevEl: '.swiper-button-prev',
                }}
                modules={[EffectCoverflow, Pagination, Navigation]}
                className="swiper-container"
                onActiveIndexChange={(swiper) => {
                  setActiveIndex(swiper.activeIndex);
                }}
                >
                {serverList.map((server, i) => {
                  // const [clientCount, setClientCount] = useState<number | null>(null);
                  // // Sử dụng useEffect để fetch dữ liệu
                  // useEffect(() => {
                  //   if (!server.enabled) return;
                    
                  //   const fetchPlayerCount = async () => {
                  //     try {
                  //       const response = await fetch(`https://${server.ip}/dynamic.json`);
                  //       const data = await response.json();
                  //       setClientCount(data.clients + 20);
                  //     } catch (error) {
                  //       console.error('Không thể tải dữ liệu từ server:', server.ip, error);
                  //     }
                  //   };
                    
                  //   fetchPlayerCount();
                  //   // Không cần interval vì chỉ fetch một lần
                  // }, [server.enabled]);
                  return <SwiperSlide 
                    key={i} 
                    onClick={() => handleSubmit(server.ip, server.enabled)}
                    className={server.enabled ? '' : 'disabled-server'}
                  >
                    {activeIndex == i ? (
                      <PlayButton 
                        serverName={server.name} 
                        onSubmit={undefined}
                        enabled={server.enabled}
                      />
                    ) : ''}
                    <img src={server.image} className={server.enabled ? '' : 'disabled-image'} />
                    {!server.enabled && (
                      <div className="server-status-overlay">
                        <Text size="large" className="server-status" weight='bold'>ĐANG ĐÓNG</Text>
                      </div>
                    )}
                    {server.enabled && mpMenu.getPlayerCount() !== null && (
                      <div className="server-status-info">
                        {/* <Text size="large" className="server-status" weight='bold'>ĐANG MỞ</Text> */}
                        <Text size="large" className="player-count">
                        •Online: <span className="secip"> {mpMenu.getPlayerCount()}</span>
                        </Text>
                      </div>
                    )}
                  </SwiperSlide>
                })}
            </Swiper>
        </Flex>
    );
});

interface SubmitControlsProps {
  onSubmit?: any;
  serverName: string;
  enabled: boolean;
}

const PlayButton = observer(function SubmitControls({ onSubmit, serverName, enabled }: SubmitControlsProps) {
  return (
    <Flex className="play-container" gap="xlarge">
      <div>
        <Text size="xlarge" className="servername" weight='bold'>{serverName}</Text>
      </div>
      {onSubmit && enabled && (
        <Button
          className="play-button"
          size="large"
          icon={Icons.playPreview}
          theme="transparent"
          onClick={onSubmit}
        />
      )}
      {!onSubmit && enabled && (
        <div className="play-button-indicator">
        </div>
      )}
    </Flex>
  )
})