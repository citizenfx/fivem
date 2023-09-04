import { Flex } from "cfx/ui/Layout/Flex/Flex";
import { observer } from "mobx-react-lite";
import { Navigation, Pagination, EffectCoverflow } from 'swiper';
import { Swiper, SwiperSlide, useSwiper, useSwiperSlide } from 'swiper/react';
import './ServerPreview.scss';
import { Text } from "cfx/ui/Text/Text";

// Import Swiper styles
import 'swiper/css';
import 'swiper/css/effect-coverflow';
import 'swiper/css/pagination';
import 'swiper/css/navigation';
import { PreviewBackgroundResponse } from "cfx/apps/mpMenu/services/serverPreview/serverPreview.service";
import { MutableRefObject, useRef, useState } from "react";

export const ServerPreview = observer(function ServerPreview({ previewBg }: { previewBg: PreviewBackgroundResponse[] | null }) {
    const swiperSlide = useSwiperSlide();
  
    return (
        <Flex className="server-preview" centered gap="xlarge">
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
                >
                {previewBg!.map((bg, i) => (
                  <SwiperSlide key={i}>
                    <p>Current slide is {swiperSlide.isActive ? 'active' : 'not active'}</p>
                    <img src={bg.url} />
                  </SwiperSlide>
                ))}
            </Swiper>
        </Flex>
    );
});