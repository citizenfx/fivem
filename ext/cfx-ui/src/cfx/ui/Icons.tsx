import { GameName } from "cfx/base/game";
import { BsCheck, BsCheckCircle, BsClock, BsCurrencyDollar, BsExclamationCircle, BsFillPersonCheckFill, BsFillPersonFill, BsGearFill, BsHeart, BsHeartFill, BsPeopleFill, BsPlay, BsQuestion, BsTags, BsTrash, BsX, BsXCircle } from "react-icons/bs";
import { FaScroll } from "react-icons/fa";
import { GiStarsStack } from "react-icons/gi";
import { MdOutlineReplay, MdTrendingUp } from "react-icons/md";
import { IoMdTrendingUp } from 'react-icons/io';
import { IoFlame } from "react-icons/io5";
import { VscListFlat } from 'react-icons/vsc';
import { BiTime } from "react-icons/bi";
import { FiExternalLink } from "react-icons/fi";
import { PiPlayCircleThin } from "react-icons/pi";

export namespace Icons {
  export const exit = <BsX style={{ fontSize: '1.75em' }} />;
  export const settings = <BsGearFill />;
  export const changelog = <FaScroll />;
  export const storymode = <BsPlay />;
  export const replayEditor = <MdOutlineReplay />;
  export const tipInfo = <BsQuestion />;
  export const remove = <BsTrash />;
  export const externalLink = <FiExternalLink />;

  export const tags = <BsTags />;

  export const checkmark = <BsCheck />;

  export const playersCount = <BsPeopleFill />;
  export const last24h = <MdTrendingUp />;

  export const statusLevelAllGood = <BsCheckCircle />;
  export const statusLevelMinor = <BsExclamationCircle />;
  export const statusLevelMajor = <BsXCircle />;

  export const favoriteInactive = <BsHeart />;
  export const favoriteActive = <BsHeartFill className="cfx-color-primary" />;

  export const account = <BsFillPersonFill />;
  export const accountLoaded = <BsFillPersonCheckFill />;

  export const serversListAll = <VscListFlat />;
  export const serversListSupporters = <BsCurrencyDollar />;
  export const serversListHistory = <BsClock />;
  export const serversListFavorites = favoriteInactive;

  export const serverLastConnected = <BiTime />;

  export const serversFeatured = <GiStarsStack style={{ fill: 'url(#pin-gradient)' }} />;
  export const serversFeaturedUnstyled = <GiStarsStack />;

  export const serverBoost = <IoMdTrendingUp style={{ fill: 'url(#boost-gradient)' }} />;
  export const serverBoostUnstyled = <IoMdTrendingUp />;
  export const serverBurst = <IoFlame style={{ fill: 'url(#burst-gradient)' }} />;

  export const playPreview = <PiPlayCircleThin />;
}

export const BrandIcon = {
  [GameName.FiveM]: (
    <svg className="brand-icon mirror">
      <use xlinkHref="#logo-gta5" />
    </svg>
  ),
  [GameName.RedM]: (
    <svg className="brand-icon">
      <use xlinkHref="#logo-rdr3" />
    </svg>
  ),
  [GameName.LibertyM]: (
    <svg className="brand-icon">
      <use xlinkHref="#logo-ny" />
    </svg>
  ),
  cfxre: (
    <svg className="brand-icon">
      <use xlinkHref="#logo-cfxre" />
    </svg>
  ),
  ROS: (
    <svg className="brand-icon" viewBox="0 0 128 128" version="1.1" xmlns="http://www.w3.org/2000/svg">
      <path d="M126.7,82.9L99.6,82.9L95.3,55.9L79.5,82.7L76.5,82.7C74.7,79.6 74,75.1 74,72.3C74,67.7 74.3,63.2 74.3,57.3C74.3,49.5 72,45.4 65.9,43.9L65.9,43.7C78.8,41.9 84.7,33.3 84.7,21.3C84.7,4.2 73.3,0.5 58.4,0.5L18.3,0.5L1.3,80.9L22.6,80.9L28.8,51.6L43,51.6C50.6,51.6 53.7,55.3 53.7,62.4C53.7,67.8 53.1,72.1 53.1,76.2C53.1,77.7 53.4,81.3 54.5,82.7L69.9,99L56.6,127.5L85,110.6L106.2,126.9L102.3,100L126.7,82.9ZM49.4,36.6L32.4,36.6L36.5,17.2L52.3,17.2C57.9,17.2 63.8,18.7 63.8,25.5C63.8,34.2 57.1,36.6 49.4,36.6M85.4,105.9L65.6,117.7L74.7,98.3L63.7,86.7L81.8,86.7L93.2,67.4L96.3,86.9L114.3,86.9L98.1,98.2L101,118L85.4,105.9Z" />
    </svg>
  ),
};
