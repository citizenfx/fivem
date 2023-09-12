import { Flex } from "cfx/ui/Layout/Flex/Flex";
import { observer } from "mobx-react-lite";
import { Navigation, Pagination, EffectCoverflow } from 'swiper';
import { Swiper, SwiperSlide, useSwiper, useSwiperSlide } from 'swiper/react';
import './ServerPreview.scss';
import { Text } from "cfx/ui/Text/Text";
import { Button, ButtonProps } from "cfx/ui/Button/Button";

// Import Swiper styles
import 'swiper/css';
import 'swiper/css/effect-coverflow';
import 'swiper/css/pagination';
import 'swiper/css/navigation';
import { PreviewBackgroundResponse } from "cfx/apps/mpMenu/services/serverPreview/serverPreview.service";
import { useState } from "react";
import { Icons } from "cfx/ui/Icons";
import { useNavigate } from "react-router-dom";
import { useService, useServiceOptional } from "cfx/base/servicesContainer";
import { IServersService } from "cfx/common/services/servers/servers.service";
import { IServersConnectService } from "cfx/common/services/servers/serversConnect.service";

export const ServerPreview = observer(function ServerPreview({ previewBg }: { previewBg: PreviewBackgroundResponse[] | null }) {
    const [activeIndex, setActiveIndex] = useState(0);
    const navigate = useNavigate();
    const ServersService = useService(IServersService);
    const ServersConnectService = useServiceOptional(IServersConnectService);

    const handleSubmit = async () => {
      // navigate(-1);
      // const server = ServersService.getServer("DESKTOP-2AP0ESB");
      ServersConnectService?.connectTo("192.168.1.13:30120")
      
    }
    return (
        <Flex className="server-preview" centered vertical gap="xlarge">
            <Flex centered>
              <img className="logo" src="https://i.imgur.com/tEwx0RH.png" />
              <div>
                <Text size="xlarge" className="title" asDiv={true}>Roleplay</Text>
                <Text size="small" className="title" asDiv={true}>Have funny moments</Text>
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
                {previewBg!.map((bg, i) => {
                  return <SwiperSlide key={i}>
                    {activeIndex == i ? <PlayButton onSubmit={handleSubmit}/> : ''}
                    <img src={bg.url} />
                  </SwiperSlide>
                })}
            </Swiper>
        </Flex>
    );
});

interface SubmitControlsProps {
  onSubmit: any
}

const PlayButton = observer(function SubmitControls({ onSubmit }: SubmitControlsProps) {
  return (
    <Flex className="play-container" gap="xlarge">
      <div>
        <Text size="normal" className="title" asDiv={true}>Welcome to</Text>
        <Text size="xlarge" className="title" weight='bold'>Devoted Roleplay</Text>
      </div>
      <Button
        className="play-button"
        size="large"
        icon={Icons.playPreview}
        theme="transparent"
        onClick={onSubmit}
      />
    </Flex>
  )
})
