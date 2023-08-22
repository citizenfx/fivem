import { Flex } from "cfx/ui/Layout/Flex/Flex";
import { observer } from "mobx-react-lite";
import { Navigation, Pagination, EffectCoverflow } from 'swiper';
import { Swiper, SwiperSlide } from 'swiper/react';
import './ServerPreview.scss';
import { Text } from "cfx/ui/Text/Text";

// Import Swiper styles
import 'swiper/css';
import 'swiper/css/effect-coverflow';
import 'swiper/css/pagination';
import 'swiper/css/navigation';

export const ServerPreview = observer(function ServerPreview() {
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
                  modifier: 1.5,
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
                <SwiperSlide>
                <img src="https://c4.wallpaperflare.com/wallpaper/133/568/943/grand-theft-auto-v-4k-background-computer-wallpaper-preview.jpg" />
                </SwiperSlide>
                <SwiperSlide>
                <img src="https://e0.pxfuel.com/wallpapers/260/655/desktop-wallpaper-gta-lambo-gta-car.jpg" />
                </SwiperSlide>
                <SwiperSlide>
                <img src="https://wallpapercrafter.com/th800/103534-GTA5-Grand-Theft-Auto-V-Grand-Theft-Auto-Hollywood.jpg" />
                </SwiperSlide>
            </Swiper>
        </Flex>
    );
});