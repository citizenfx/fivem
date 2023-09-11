/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include <CommCtrl.h>

#ifdef LAUNCHER_PERSONALITY_MAIN
#include <shobjidl.h>

#include "launcher.rc.h"

#include <ShellScalingApi.h>

#include <winrt/Windows.Storage.Streams.h>

#include "CitiLaunch/BackdropBrush.g.h"
#include "winrt/Microsoft.Graphics.Canvas.Effects.h"

#include <DirectXMath.h>
#include <roapi.h>

#include <CfxState.h>
#include <HostSharedData.h>

#include <boost/algorithm/string.hpp>

#include <d2d1effects.h>
#include <d2d1_1.h>
#pragma comment(lib, "dxguid.lib")

#include <windows.graphics.effects.h>
#include <windows.graphics.effects.interop.h>

#pragma comment(lib, "runtimeobject.lib")
#pragma comment(lib, "delayimp.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "shcore.lib")

using namespace ABI::Windows::Graphics::Effects;

struct CompositionEffect : winrt::implements
	<
		CompositionEffect,
		winrt::Windows::Graphics::Effects::IGraphicsEffectSource, 
		winrt::Windows::Graphics::Effects::IGraphicsEffect,
		ABI::Windows::Graphics::Effects::IGraphicsEffectD2D1Interop
	>
{
	CompositionEffect(const GUID& effectId)
	{
		m_effectId = effectId;
	}

	winrt::hstring Name()
	{
		return m_name;
	}

	void Name(winrt::hstring const& name)
	{
		m_name = name;
	}

	template<typename T>
	void SetProperty(const std::string& name, const T& value, GRAPHICS_EFFECT_PROPERTY_MAPPING mapping = GRAPHICS_EFFECT_PROPERTY_MAPPING_DIRECT)
	{
		m_properties.emplace_back(name, winrt::box_value(value), mapping);
	}

	template<int N>
	void SetProperty(const std::string& name, const float (&value)[N], GRAPHICS_EFFECT_PROPERTY_MAPPING mapping = GRAPHICS_EFFECT_PROPERTY_MAPPING_DIRECT)
	{
		m_properties.emplace_back(name, winrt::Windows::Foundation::PropertyValue::CreateSingleArray(winrt::array_view<const float>{ (float*)&value, (float*)&value + N }), mapping);
	}

	template<>
	void SetProperty(const std::string& name, const winrt::Windows::UI::Color& color, GRAPHICS_EFFECT_PROPERTY_MAPPING mapping)
	{
		float values[] = { color.R / 255.0f, color.G / 255.0f, color.B / 255.0f, color.A / 255.0f };
		SetProperty(name, values, mapping);
	}

	template<>
	void SetProperty(const std::string& name, const winrt::Microsoft::Graphics::Canvas::Effects::Matrix5x4& value, GRAPHICS_EFFECT_PROPERTY_MAPPING mapping)
	{
		float mat[5 * 4];
		memcpy(mat, &value, sizeof(mat));
		SetProperty(name, mat, mapping);
	}

	template<>
	void SetProperty(const std::string& name, const winrt::Windows::Foundation::Numerics::float3x2& value, GRAPHICS_EFFECT_PROPERTY_MAPPING mapping)
	{
		float mat[3 * 2];
		memcpy(mat, &value, sizeof(mat));
		SetProperty(name, mat, mapping);
	}

	void AddSource(const winrt::Windows::Graphics::Effects::IGraphicsEffectSource& source)
	{
		m_sources.push_back(source);
	}

	virtual HRESULT __stdcall GetEffectId(GUID* id) override
	{
		*id = m_effectId;
		return S_OK;
	}

	virtual HRESULT __stdcall GetNamedPropertyMapping(LPCWSTR name, UINT* index, GRAPHICS_EFFECT_PROPERTY_MAPPING* mapping) override
	{
		auto nname = ToNarrow(name);

		auto entry = std::find_if(m_properties.begin(), m_properties.end(), [&nname](const auto& property)
		{
			return nname == std::get<0>(property);
		});

		if (entry != m_properties.end())
		{
			*index = entry - m_properties.begin();
			*mapping = std::get<2>(*entry);
			return S_OK;
		}

		return HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
	}

	virtual HRESULT __stdcall GetPropertyCount(UINT* count) override
	{
		*count = m_properties.size();
		return S_OK;
	}

	virtual HRESULT __stdcall GetProperty(UINT index, ABI::Windows::Foundation::IPropertyValue** value) override
	{
		std::get<1>(m_properties[index]).as<ABI::Windows::Foundation::IPropertyValue>().copy_to(value);
		return S_OK;
	}

	virtual HRESULT __stdcall GetSource(UINT index, ABI::Windows::Graphics::Effects::IGraphicsEffectSource** source) override
	{
		m_sources[index].as<ABI::Windows::Graphics::Effects::IGraphicsEffectSource>().copy_to(source);
		return S_OK;
	}

	virtual HRESULT __stdcall GetSourceCount(UINT* count) override
	{
		*count = UINT(m_sources.size());
		return S_OK;
	}

private:
	GUID m_effectId;

	winrt::hstring m_name = L"";

	std::vector<std::tuple<std::string, winrt::Windows::Foundation::IInspectable, GRAPHICS_EFFECT_PROPERTY_MAPPING>> m_properties;
	std::vector<winrt::Windows::Graphics::Effects::IGraphicsEffectSource> m_sources;
};

static class DPIScaler
{
public:
	DPIScaler()
	{
		// Default DPI is 96 (100%)
		dpiX = 96;
		dpiY = 96;
	}

	void SetScale(UINT dpiX, UINT dpiY)
	{
		this->dpiX = dpiX;
		this->dpiY = dpiY;
	}

	int ScaleX(int x)
	{
		return MulDiv(x, dpiX, 96);
	}

	int ScaleY(int y)
	{
		return MulDiv(y, dpiY, 96);
	}

private:
	UINT dpiX, dpiY;
} g_dpi;

using namespace winrt::Windows::UI;
using namespace winrt::Windows::UI::Composition;
using namespace winrt::Windows::UI::Xaml::Hosting;
using namespace winrt::Windows::Foundation::Numerics;

struct TenUI
{
	DesktopWindowXamlSource uiSource{ nullptr };

	winrt::Windows::UI::Xaml::UIElement snailContainer{ nullptr };
	winrt::Windows::UI::Xaml::Controls::TextBlock topStatic{ nullptr };
	winrt::Windows::UI::Xaml::Controls::TextBlock bottomStatic{ nullptr };
	winrt::Windows::UI::Xaml::Controls::ProgressBar progressBar{ nullptr };
};

//static thread_local struct  
static struct
{
	HWND rootWindow;
	HWND topStatic;
	HWND bottomStatic;
	HWND progressBar;
	HWND cancelButton;

	HWND tenWindow;

	UINT taskbarMsg;

	bool tenMode;
	bool canceled;

	std::unique_ptr<TenUI> ten;

	ITaskbarList3* tbList;

	wchar_t topText[512];
	wchar_t bottomText[512];
} g_uui;

HWND UI_GetWindowHandle()
{
	return g_uui.rootWindow;
}

HFONT UI_CreateScaledFont(int cHeight, int cWidth, int cEscapement, int cOrientation, int cWeight, DWORD bItalic,
	DWORD bUnderline, DWORD bStrikeOut, DWORD iCharSet, DWORD iOutPrecision, DWORD iClipPrecision,
	DWORD iQuality, DWORD iPitchAndFamily, LPCWSTR pszFaceName)
{
	LOGFONT logFont;
	
	memset(&logFont, 0, sizeof(LOGFONT));
	logFont.lfHeight = g_dpi.ScaleY(cHeight);
	logFont.lfWidth = cWidth;
	logFont.lfEscapement = cEscapement;
	logFont.lfOrientation = cOrientation;
	logFont.lfWeight = cWeight;
	logFont.lfItalic = bItalic;
	logFont.lfUnderline = bUnderline;
	logFont.lfStrikeOut = bStrikeOut;
	logFont.lfCharSet = iCharSet;
	logFont.lfOutPrecision = 8;
	logFont.lfClipPrecision = iClipPrecision;
	logFont.lfQuality = iQuality;
	logFont.lfPitchAndFamily = iPitchAndFamily;
	wcscpy_s(logFont.lfFaceName, pszFaceName);
	return CreateFontIndirect(&logFont);
}

static std::wstring g_mainXaml = LR"(
<Grid
    xmlns="http://schemas.microsoft.com/winfx/2006/xaml/presentation"
    xmlns:x="http://schemas.microsoft.com/winfx/2006/xaml"
    xmlns:d="http://schemas.microsoft.com/expression/blend/2008"
    xmlns:mc="http://schemas.openxmlformats.org/markup-compatibility/2006"
	xmlns:local="using:CitiLaunch"
    mc:Ignorable="d">
	<Border CornerRadius="10">
		<Grid Width="525" Height="525">
			<Grid.Resources>
				<!--<ThemeShadow x:Name="SharedShadow">
				</ThemeShadow>-->
			</Grid.Resources>
			<Grid x:Name="BackdropGrid"/>
			<SwapChainPanel x:Name="Overlay" />
			<StackPanel Orientation="Vertical" VerticalAlignment="Center">)"
	#if defined(GTA_FIVE)
		R"(
				<Viewbox Height="150" Margin="0,0,0,15" RenderTransformOrigin="0.5,0.5">
					<Path Data="M 658.999 8.750 C 658.998 21.306, 657.323 37.217, 654.517 51.316 C 652.239 62.764, 644.588 86.096, 640.520 94 C 630.495 113.478, 609.265 131.896, 568.500 156.481 C 553.363 165.610, 532.419 189.341, 522.958 208.083 C 517.984 217.936, 513.239 230.948, 511.073 240.673 C 508.960 250.157, 507.026 272.573, 507.010 287.759 L 507 297.017 501.750 296.538 C 498.863 296.274, 492.012 295.804, 486.528 295.493 C 470.185 294.568, 457.496 292.439, 393.002 279.804 L 331.500 267.755 316 267.699 C 303.734 267.655, 298.400 268.094, 290.435 269.803 C 247.853 278.940, 216.239 301.150, 195.069 336.803 C 164.344 388.547, 141.581 464.782, 132.305 547 C 129.819 569.037, 129.820 627.951, 132.306 646.334 C 137.149 682.134, 144.747 706.067, 157.468 725.582 C 166.539 739.498, 176.175 747.360, 190.758 752.743 C 203.937 757.607, 230.166 756.704, 254.412 750.551 C 281.298 743.727, 303.075 731.293, 328 708.531 C 356.602 682.412, 375.864 667.406, 394 657.115 C 419.890 642.424, 445.600 633.583, 476.203 628.846 C 496.461 625.711, 526.561 625.715, 546.856 628.855 C 600.541 637.161, 637.684 656.345, 687.307 701.395 C 714.897 726.443, 729.122 736.126, 751.962 745.406 C 773.106 753.998, 810.460 758.463, 826.936 754.369 C 846.837 749.424, 861.397 736.544, 872.004 714.500 C 880.962 695.884, 885.671 679.182, 890.048 650.500 C 892.787 632.555, 893.827 587.941, 892.068 563.798 C 884.704 462.676, 850.394 357.615, 809.708 311.601 C 797.472 297.763, 769.854 280.364, 750.116 274.060 C 741.469 271.298, 725.333 268.272, 715 267.475 C 701.919 266.467, 690.928 268.076, 630 279.922 C 556.774 294.159, 548.931 295.385, 525.597 296.246 L 518.694 296.500 519.352 276.500 C 519.714 265.500, 520.469 253.376, 521.028 249.557 C 522.311 240.813, 527.763 224.413, 532.570 214.840 C 541.894 196.275, 558.156 177.081, 572.129 168.150 C 602.785 148.555, 612.506 141.824, 621.428 134.014 C 634.364 122.690, 638.709 118.083, 644.448 109.601 C 650.805 100.209, 652.935 95.720, 658.498 80 C 665.648 59.793, 669.297 39.770, 670.569 13.750 L 671.241 -0 665.121 -0 L 659 0 658.999 8.750 M 300.500 281.674 C 282.622 283.905, 262.134 291.555, 246.500 301.837 C 238.536 307.075, 222.091 322.581, 215.042 331.500 C 199.421 351.263, 180.695 393.046, 167.535 437.500 C 143.940 517.204, 136.935 607.417, 149.577 668.778 C 152.289 681.946, 157.494 698.087, 161.722 706.447 C 168.737 720.319, 180.620 733.552, 190.506 738.500 L 196.500 741.500 215.500 741.471 C 241.556 741.431, 256.298 738.324, 275.929 728.733 C 292.287 720.741, 300.823 714.381, 326 691.422 C 356.716 663.412, 382.032 646.217, 411 633.690 C 448.940 617.283, 494.504 609.585, 531 613.417 C 578.549 618.408, 614.374 630.709, 650.500 654.448 C 665.687 664.427, 678.775 674.835, 698.978 693 C 722.368 714.029, 731.657 720.842, 747.353 728.478 C 768.127 738.585, 786.945 742.455, 812.340 741.842 C 825.576 741.522, 826.891 741.310, 832.500 738.589 C 840.146 734.880, 852.393 722.701, 857.287 713.939 C 881.134 671.247, 886.332 595.319, 871.490 506.500 C 862.091 450.254, 846.274 400.359, 824.461 358.141 C 815.251 340.317, 808.130 330.261, 796.294 318.366 C 775.848 297.818, 751.472 285.614, 723.520 281.934 C 704.798 279.468, 695.305 280.650, 628 293.825 C 545.955 309.886, 549.175 309.455, 511.500 309.418 C 474.404 309.382, 475.161 309.481, 400.765 294.937 C 331.303 281.357, 328.916 280.973, 315.151 281.150 C 308.743 281.232, 302.150 281.468, 300.500 281.674 M 675 354.115 C 667.570 355.334, 657.559 360.388, 652.446 365.501 C 647.153 370.795, 642.251 380.958, 641.166 388.891 C 638.718 406.797, 647.666 424.819, 662.594 432.045 C 676.251 438.657, 690.103 438.376, 703.395 431.217 C 713.457 425.798, 720.819 415.006, 723.205 402.174 C 725.394 390.403, 721.315 376.184, 713.058 366.803 C 704.656 357.258, 688.556 351.891, 675 354.115 M 298.894 360.114 C 294.301 361.800, 294 363.997, 294 395.892 C 294 423.490, 293.865 425.812, 292.171 427.345 C 290.557 428.805, 286.910 429, 261.139 429 C 227.904 429, 225.569 429.394, 224.510 435.174 C 224.174 437.003, 224.035 455.247, 224.200 475.715 C 224.475 509.861, 224.647 513.078, 226.284 514.715 C 227.907 516.339, 230.889 516.526, 259.252 516.792 C 285.872 517.041, 290.688 517.313, 292.165 518.650 C 293.723 520.059, 293.926 523.349, 294.198 551.574 C 294.473 580.104, 294.661 583.092, 296.286 584.716 C 297.940 586.368, 301.015 586.500, 338 586.500 C 374.986 586.500, 378.060 586.369, 379.714 584.716 C 381.339 583.092, 381.526 580.119, 381.792 551.748 C 382.042 525.033, 382.310 520.315, 383.661 518.822 C 385.085 517.249, 388.275 517.052, 416.584 516.790 C 445.107 516.526, 448.092 516.339, 449.716 514.715 C 451.355 513.075, 451.523 509.835, 451.785 474.757 C 451.984 448.063, 451.730 435.763, 450.941 433.856 C 450.277 432.255, 448.550 430.690, 446.756 430.065 C 444.866 429.406, 432.577 429, 414.506 429 C 387.491 429, 385.186 428.863, 383.655 427.171 C 382.195 425.558, 382 421.922, 382 396.320 C 382 378.243, 381.596 366.138, 380.927 364.222 C 380.211 362.166, 378.834 360.789, 376.778 360.073 C 372.876 358.712, 302.612 358.750, 298.894 360.114 M 672.748 368.427 C 661.811 372.330, 654 383.235, 654 394.602 C 654 402.982, 655.798 407.618, 661.445 413.797 C 667.966 420.932, 672.436 422.950, 681.782 422.978 C 693.209 423.013, 701.488 418.309, 707.041 408.626 C 709.627 404.117, 709.980 402.536, 709.969 395.500 C 709.960 389.921, 709.405 386.288, 708.137 383.495 C 705.783 378.314, 695.982 369.367, 691.208 368.041 C 686.121 366.628, 677.269 366.813, 672.748 368.427 M 309.655 374.829 C 308.190 376.447, 308 380.211, 308 407.606 C 308 433.999, 307.771 438.882, 306.443 440.777 C 304.890 442.995, 304.814 443, 273.098 443 C 243.572 443, 241.192 443.130, 239.655 444.829 C 238.203 446.434, 238 449.903, 238 473.174 C 238 497.592, 238.145 499.821, 239.829 501.345 C 241.448 502.810, 245.227 503, 272.829 503 C 302.667 503, 304.086 503.086, 306 505 C 307.915 506.915, 308 508.333, 308 538.345 C 308 567.442, 308.131 569.809, 309.829 571.345 C 311.434 572.797, 314.903 573, 338.174 573 C 362.592 573, 364.821 572.855, 366.345 571.171 C 367.810 569.552, 368 565.773, 368 538.171 C 368 508.333, 368.086 506.914, 370 505 C 371.915 503.085, 373.333 503, 403.345 503 C 432.442 503, 434.809 502.869, 436.345 501.171 C 437.797 499.566, 438 496.097, 438 472.826 C 438 448.408, 437.855 446.179, 436.171 444.655 C 434.551 443.188, 430.742 443, 402.728 443 C 371.192 443, 371.110 442.994, 369.557 440.777 C 368.229 438.882, 368 433.982, 368 407.432 C 368 378.551, 367.868 376.191, 366.171 374.655 C 364.566 373.203, 361.097 373, 337.826 373 C 313.408 373, 311.179 373.145, 309.655 374.829 M 602.500 428.687 C 585.931 430.941, 573.323 441.091, 567.950 456.500 C 566.250 461.374, 565.925 464.283, 566.220 472 C 566.544 480.505, 566.967 482.297, 570.258 489.104 C 574.448 497.770, 579.153 502.547, 587.720 506.833 C 602.842 514.398, 617.908 513.320, 632.504 503.629 C 640.937 498.030, 647.381 486.216, 648.656 474.020 C 649.929 461.838, 643.867 446.115, 635.031 438.680 C 626.792 431.747, 612.433 427.336, 602.500 428.687 M 750 429.132 C 732.753 432.014, 720.103 444.397, 716.435 461.990 C 714.503 471.254, 715.792 479.942, 720.542 489.685 C 730.443 509.993, 755.501 517.658, 776.743 506.877 C 788.426 500.947, 795.570 490.984, 798.036 477.184 C 800.173 465.226, 795.878 450.405, 787.630 441.276 C 779.392 432.157, 763.189 426.928, 750 429.132 M 598.764 442.996 C 590.493 445.486, 582.034 454.523, 579.987 463.054 C 578.485 469.317, 579.311 477.728, 581.933 482.869 C 584.327 487.562, 591.731 494.443, 596.555 496.458 C 601.328 498.453, 612.954 498.464, 617.705 496.479 C 623.575 494.026, 629.617 488.410, 632.405 482.814 C 635.678 476.247, 636.021 464.844, 633.137 458.495 C 630.752 453.245, 621.354 444.722, 616.272 443.200 C 611.776 441.853, 602.904 441.749, 598.764 442.996 M 747.748 443.427 C 740.536 446, 733.699 452.690, 730.531 460.272 C 728.699 464.657, 728.540 473.734, 730.200 479.272 C 731.722 484.354, 740.245 493.752, 745.495 496.137 C 750.605 498.458, 762.554 498.628, 767.782 496.454 C 772.670 494.421, 780.752 486.528, 783.093 481.500 C 785.276 476.813, 785.611 466.817, 783.794 460.616 C 782.147 455, 772 444.853, 766.384 443.206 C 761.030 441.637, 752.479 441.738, 747.748 443.427 M 675 504.115 C 667.570 505.334, 657.559 510.388, 652.446 515.501 C 640.598 527.349, 637.470 547.723, 645.058 563.622 C 649.040 571.964, 654.789 577.786, 663.075 581.865 C 676.029 588.243, 687.307 588.321, 700.549 582.124 C 709.019 578.160, 715.045 572.381, 718.923 564.505 C 725.134 551.889, 725.323 538.900, 719.475 526.539 C 711.870 510.463, 693.318 501.109, 675 504.115 M 672.748 518.427 C 664.821 521.256, 657.010 529.554, 654.886 537.404 C 653.378 542.974, 654.295 552.708, 656.782 557.529 C 659.506 562.810, 665.465 568.423, 671 570.923 C 676.172 573.260, 685.110 573.646, 691.272 571.800 C 696.354 570.278, 705.752 561.755, 708.137 556.505 C 710.224 551.911, 710.594 541.679, 708.894 535.557 C 707.395 530.158, 696.993 519.850, 691.384 518.206 C 686.030 516.637, 677.479 516.738, 672.748 518.427" Fill="#205a53" Stretch="Fill"></Path>
					<Viewbox.RenderTransform>
						<ScaleTransform ScaleX="-1" />
					</Viewbox.RenderTransform>)"
	#elif defined(IS_RDR3)
		R"(
				<Viewbox Height="150" Margin="0,0,0,15">
					<Grid>
					<Path Data="F1 M 38.56,38.56 L 779.52,38.56 779.52,1019.52 38.56,1019.52 z"  Fill="#00000000" />
					<Path Data="F1 M 153.23,78.44 L 154.67,77.16 153.23,75.72 153.23,78.44 153.23,78.44 z"  Fill="#ffffffff" />
					<Path Data="F1 M 677.12,48.82 L 523.2,98.61 516.32,118.63 673.43,67.71 677.12,48.82 677.12,48.82 z"  Fill="#ffffffff" />
					<Path Data="F1 M 666.07,105.5 L 668.63,92.37 507.35,144.73 502.7,158.34 666.07,105.5 666.07,105.5 z"  Fill="#ffffffff" />
					<Path Data="F1 M 0,0 L -13.94,40.99 -32.52,105.83 -42.61,153.07 116.91,176.77 134.69,107.28 166.24,-53.8
						 0,0 0.16,0 z" RenderTransform="1,0,0,1,496.62,175.63" Fill="#ffffffff" />
					<Path Data="F1 M 670.55,38.73 L 543.7,38.73 527.85,84.84 670.55,38.73 z"  Fill="#ffffffff" />
					<Path Data="F1 M 311.47,167.46 L 152.43,218.86 151.95,224.46 151.95,236.79 310.51,185.55 311.47,167.46 z"  Fill="#ffffffff" />
					<Path Data="F1 M 308.91,221.26 L 309.55,209.09 151.95,260.01 151.95,272.18 308.91,221.26 308.91,221.26 z"  Fill="#ffffffff" />
					<Path Data="F1 M 0,0 L -9.45,-0.96 19.22,-146.18 -133.89,-168.92 -144.78,-118.16 -164.64,-6.72 -266.82,-6.72
						 -245.52,-296.05 -245.52,-404.93 -244.72,-425.59 -401.04,-375.15 -401.04,-265.63 -405.2,-256.34 -404.88,-247.7
						 -409.05,-182.05 -412.09,-168.76 -411.77,-133.06 -411.77,358.34 94.18,358.34 94.18,326.48 116.76,-5.28
						 34.44,0 0,0 0,0 z
						M -25.14,211.04 L -272.27,211.04 -264.26,171.33 -264.26,143.31 -272.27,124.73 -25.14,124.73 -27.87,163.16
						 -25.14,211.04 z" RenderTransform="1,0,0,1,552.99,662.7" Fill="#ffffffff" />
					<Path Data="F1 M 0,0 L 2.88,-108.56 -156.31,-113.84 -155.83,-101.99 -157.12,-79.41 -157.12,37.47 -158.24,51.08
						 0,0 0,0 z" RenderTransform="1,0,0,1,311.79,155.13" Fill="#ffffffff" />
					</Grid>
	)"
	#elif defined(GTA_NY)
									 R"(
				<Viewbox Height="150" Margin="0,0,0,15">
					<Grid>
					<Path Data="M26,145L54.571,145C54.952,144.905 55.143,144.714 55.143,144.429L55.143,69C43.714,57.286 33.905,47.476 25.714,39.571L25.429,39.571L25.429,144.429C25.524,144.81 25.714,145 26,145ZM54.857,57.857L55.143,57.857L55.143,54.143C43.714,42.429 33.905,32.619 25.714,24.714L25.429,24.714L25.429,28.429C36.857,40.048 46.667,49.857 54.857,57.857ZM54.857,43L55.143,43L55.143,31.571C46.857,23 38,14.143 28.571,5L26,5C25.619,5 25.429,5.19 25.429,5.571L25.429,13.571C36.857,25.19 46.667,35 54.857,43ZM57.714,30.429L124,30.429C124.381,30.333 124.571,30.143 124.571,29.857L124.571,5.571C124.571,5.19 124.381,5 124,5L32.571,5L32.571,5.286C41.714,14.619 50.095,23 57.714,30.429Z"  Fill="#ffffffff" />
					</Grid>
	)"
	#endif
	R"(         </Viewbox>
				<TextBlock x:Name="static1" Text=" " TextAlignment="Center" Foreground="#ffffffff" FontSize="24" />
				<Grid Margin="0,15,0,15">
					<ProgressBar x:Name="progressBar" Foreground="#00403a" Width="250" Height="10">
						<ProgressBar.Clip>
							<RectangleGeometry RadiusX="10" RadiusY="10" Rect="0,0,300,10"/>
						</ProgressBar.Clip>
					</ProgressBar>
				</Grid>
				<TextBlock x:Name="static2" Text=" " TextAlignment="Center" Foreground="#ffeeeeee" FontSize="18" />
				<StackPanel Orientation="Horizontal" HorizontalAlignment="Center" x:Name="snailContainer" Visibility="Collapsed">
					<TextBlock TextAlignment="Center" Foreground="#ddeeeeee" FontSize="14" Width="430" TextWrapping="Wrap">
						🐌 RedM game storage downloads are peer-to-peer and may be slower than usual downloads. Please be patient.
					</TextBlock>
				</StackPanel>
			</StackPanel>
		</Grid>
	</Border>
</Grid>
)";

struct BackdropBrush : winrt::CitiLaunch::implementation::BackdropBrushT<BackdropBrush>
{
	BackdropBrush() = default;

	void OnConnected();
	void OnDisconnected();

	winrt::Windows::UI::Composition::CompositionPropertySet ps{ nullptr };
};

void BackdropBrush::OnConnected()
{
	if (!CompositionBrush())
	{
		//
		// !NOTE! if trying to change the following code (add extra effects, change effects, etc.)
		// 
		// CLSIDs and properties are from Win2D:
		//   https://github.com/microsoft/Win2D/tree/99ce19f243c6a6332f0ea312cd29fc3c785a540b/winrt/lib/effects/generated
		//
		// The .h files show the CLSID used, the .cpp files the properties with names, type, mapping and default values.
		// 
		// Properties *have* to be set in the original order - initial deserialization (at least in wuceffects.dll 10.0.22000)
		// will check all properties before checking the name mapping. Also, `Source` properties are mapped to the AddSource
		// list, instead of being a real property.
		//
		auto effect = CompositionEffect(CLSID_D2D1Flood);

#ifdef GTA_FIVE
		effect.SetProperty("Color", winrt::Windows::UI::ColorHelper::FromArgb(255, 0x16, 0x19, 0x23));
#elif defined(IS_RDR3)
		effect.SetProperty("Color", winrt::Windows::UI::ColorHelper::FromArgb(255, 186, 2, 2));
#elif defined(GTA_NY)
		effect.SetProperty("Color", winrt::Windows::UI::ColorHelper::FromArgb(255, 0x4D, 0xA6, 0xD3));
#endif

		winrt::Windows::UI::Composition::CompositionEffectSourceParameter sp{ L"layer" };
		winrt::Windows::UI::Composition::CompositionEffectSourceParameter sp2{ L"rawImage" };

		auto mat2d = winrt::Windows::Foundation::Numerics::float3x2{};

		using namespace DirectX;
		auto matrix = XMMatrixTransformation2D(XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f), 0.0f, XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f), XMVectorSet(0.5f, 0.5f, 0.0f, 0.0f), 0.2, XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f));
		XMStoreFloat3x2(&mat2d, matrix);

		auto layer = CompositionEffect(CLSID_D2D12DAffineTransform);
		layer.AddSource(sp2);
		layer.Name(L"xform");

		layer.SetProperty("InterpolationMode", uint32_t(D2D1_2DAFFINETRANSFORM_INTERPOLATION_MODE_LINEAR));
		layer.SetProperty("BorderMode", uint32_t(D2D1_BORDER_MODE_SOFT));
		layer.SetProperty("TransformMatrix", mat2d);
		layer.SetProperty("Sharpness", 0.0f);

		auto mat = winrt::Microsoft::Graphics::Canvas::Effects::Matrix5x4();
		memset(&mat, 0, sizeof(mat));
		mat.M44 = 1.0f;

#ifdef GTA_FIVE
		mat.M11 = 1.0f;
		mat.M22 = 1.0f;
		mat.M33 = 1.0f;
		mat.M44 = 0.03f;
#elif defined(IS_RDR3) || defined(GTA_NY)
		mat.M11 = 1.0f;
		mat.M22 = 1.0f;
		mat.M33 = 1.0f;
		mat.M44 = 0.15f;
#endif

		auto layerColor = CompositionEffect(CLSID_D2D1ColorMatrix);
		layerColor.AddSource(layer);
		layerColor.SetProperty("ColorMatrix", mat);
		layerColor.SetProperty("AlphaMode", uint32_t(D2D1_COLORMATRIX_ALPHA_MODE_PREMULTIPLIED), GRAPHICS_EFFECT_PROPERTY_MAPPING_COLORMATRIX_ALPHA_MODE);
		layerColor.SetProperty("ClampOutput", false);

		auto compEffect = CompositionEffect(CLSID_D2D1Composite);
		compEffect.SetProperty("Mode", uint32_t(D2D1_COMPOSITE_MODE_SOURCE_OVER));
		compEffect.AddSource(effect);
		compEffect.AddSource(layerColor);

		auto hRsc = FindResource(GetModuleHandle(NULL), MAKEINTRESOURCE(IDM_BACKDROP), L"MEOW");
		auto resSize = SizeofResource(GetModuleHandle(NULL), hRsc);
		auto resData = LoadResource(GetModuleHandle(NULL), hRsc);

		auto resPtr = static_cast<const uint8_t*>(LockResource(resData));

		auto iras = winrt::Windows::Storage::Streams::InMemoryRandomAccessStream();
		auto dw = winrt::Windows::Storage::Streams::DataWriter{ iras };
		dw.WriteBytes(winrt::array_view<const uint8_t>{resPtr, resPtr + resSize});

		auto iao = dw.StoreAsync();
		while (iao.Status() != winrt::Windows::Foundation::AsyncStatus::Completed)
		{
			Sleep(0);
		}

		iras.Seek(0);

		auto surf = winrt::Windows::UI::Xaml::Media::LoadedImageSurface::StartLoadFromStream(iras);

		auto cb = winrt::Windows::UI::Xaml::Window::Current().Compositor().CreateSurfaceBrush();
		cb.Surface(surf);
		//cb.Stretch(winrt::Windows::UI::Composition::CompositionStretch::UniformToFill);
		cb.Stretch(winrt::Windows::UI::Composition::CompositionStretch::None);

		auto ef = winrt::Windows::UI::Xaml::Window::Current().Compositor().CreateEffectFactory(compEffect, { L"xform.TransformMatrix" });
		auto eb = ef.CreateBrush();
		eb.SetSourceParameter(L"rawImage", cb);

		using namespace std::chrono_literals;

		auto kfa = winrt::Windows::UI::Xaml::Window::Current().Compositor().CreateVector2KeyFrameAnimation();
		kfa.InsertKeyFrame(0.0f, { 0.0f, 0.0f });
		kfa.InsertKeyFrame(0.25f, { 0.0f, -300.0f }, winrt::Windows::UI::Xaml::Window::Current().Compositor().CreateLinearEasingFunction());
		kfa.InsertKeyFrame(0.5f, { -300.0f, -300.0f }, winrt::Windows::UI::Xaml::Window::Current().Compositor().CreateLinearEasingFunction());
		kfa.InsertKeyFrame(0.75f, { -300.0f, 0.0f }, winrt::Windows::UI::Xaml::Window::Current().Compositor().CreateLinearEasingFunction());
		kfa.InsertKeyFrame(1.0f, { 0.0f, 0.0f }, winrt::Windows::UI::Xaml::Window::Current().Compositor().CreateLinearEasingFunction());
		kfa.Duration(60s);
		kfa.IterationBehavior(winrt::Windows::UI::Composition::AnimationIterationBehavior::Forever);
		kfa.Target(L"xlate");

		auto ag = winrt::Windows::UI::Xaml::Window::Current().Compositor().CreateAnimationGroup();
		ag.Add(kfa);

		ps = winrt::Windows::UI::Xaml::Window::Current().Compositor().CreatePropertySet();
		ps.InsertVector2(L"xlate", { 0.0f, 0.0f });
		ps.StartAnimationGroup(ag);

		auto ca = winrt::Windows::UI::Xaml::Window::Current().Compositor().CreateExpressionAnimation();
		ca.SetReferenceParameter(L"ps", ps);
		ca.SetMatrix3x2Parameter(L"rot", mat2d);
		ca.Expression(L"Matrix3x2.CreateFromTranslation(ps.xlate) * rot");

		eb.StartAnimation(L"xform.TransformMatrix", ca);

		CompositionBrush(eb);
	}
}

void BackdropBrush::OnDisconnected()
{
	if (CompositionBrush())
	{
		CompositionBrush(nullptr);
	}
}

#include <wrl.h>
#include <d3d11.h>
#include <dxgi1_4.h>

#include <windows.ui.xaml.media.dxinterop.h>

using Microsoft::WRL::ComPtr;

const BYTE g_PixyShader[] =
{
     68,  88,  66,  67, 115,  61, 
    165, 134, 202, 176,  67, 148, 
    204, 160, 214, 207, 231, 188, 
    224, 101,   1,   0,   0,   0, 
     48,  10,   0,   0,   5,   0, 
      0,   0,  52,   0,   0,   0, 
     36,   1,   0,   0, 124,   1, 
      0,   0, 176,   1,   0,   0, 
    180,   9,   0,   0,  82,  68, 
     69,  70, 232,   0,   0,   0, 
      1,   0,   0,   0,  68,   0, 
      0,   0,   1,   0,   0,   0, 
     28,   0,   0,   0,   0,   4, 
    255, 255,   0,   1,   0,   0, 
    192,   0,   0,   0,  60,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   1,   0, 
      0,   0,   1,   0,   0,   0, 
     80, 115,  67,  98, 117, 102, 
      0, 171,  60,   0,   0,   0, 
      2,   0,   0,   0,  92,   0, 
      0,   0,  16,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0, 140,   0,   0,   0, 
      0,   0,   0,   0,   8,   0, 
      0,   0,   0,   0,   0,   0, 
    152,   0,   0,   0,   0,   0, 
      0,   0, 168,   0,   0,   0, 
      8,   0,   0,   0,   4,   0, 
      0,   0,   2,   0,   0,   0, 
    176,   0,   0,   0,   0,   0, 
      0,   0, 105,  82, 101, 115, 
    111, 108, 117, 116, 105, 111, 
    110,   0,   1,   0,   3,   0, 
      1,   0,   2,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
    105,  84, 105, 109, 101,   0, 
    171, 171,   0,   0,   3,   0, 
      1,   0,   1,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
     77, 105,  99, 114, 111, 115, 
    111, 102, 116,  32,  40,  82, 
     41,  32,  72,  76,  83,  76, 
     32,  83, 104,  97, 100, 101, 
    114,  32,  67, 111, 109, 112, 
    105, 108, 101, 114,  32,  49, 
     48,  46,  49,   0,  73,  83, 
     71,  78,  80,   0,   0,   0, 
      2,   0,   0,   0,   8,   0, 
      0,   0,  56,   0,   0,   0, 
      0,   0,   0,   0,   1,   0, 
      0,   0,   3,   0,   0,   0, 
      0,   0,   0,   0,  15,   0, 
      0,   0,  68,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   3,   0,   0,   0, 
      1,   0,   0,   0,   3,   3, 
      0,   0,  83,  86,  95,  80, 
     79,  83,  73,  84,  73,  79, 
     78,   0,  84,  69,  88,  67, 
     79,  79,  82,  68,   0, 171, 
    171, 171,  79,  83,  71,  78, 
     44,   0,   0,   0,   1,   0, 
      0,   0,   8,   0,   0,   0, 
     32,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      3,   0,   0,   0,   0,   0, 
      0,   0,  15,   0,   0,   0, 
     83,  86,  95,  84,  65,  82, 
     71,  69,  84,   0, 171, 171, 
     83,  72,  68,  82, 252,   7, 
      0,   0,  64,   0,   0,   0, 
    255,   1,   0,   0,  89,   0, 
      0,   4,  70, 142,  32,   0, 
      0,   0,   0,   0,   1,   0, 
      0,   0,  98,  16,   0,   3, 
     50,  16,  16,   0,   1,   0, 
      0,   0, 101,   0,   0,   3, 
    242,  32,  16,   0,   0,   0, 
      0,   0, 104,   0,   0,   2, 
      5,   0,   0,   0,  56,   0, 
      0,  11,  50,   0,  16,   0, 
      0,   0,   0,   0, 166, 138, 
     32,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   2,  64, 
      0,   0, 205, 204, 204,  61, 
     66,  96, 101,  63,   0,   0, 
      0,   0,   0,   0,   0,   0, 
     77,   0,   0,   6,  18,   0, 
     16,   0,   0,   0,   0,   0, 
      0, 208,   0,   0,  10,   0, 
     16,   0,   0,   0,   0,   0, 
     50,   0,   0,  15, 194,   0, 
     16,   0,   0,   0,   0,   0, 
     86,  17,  16,   0,   1,   0, 
      0,   0,   2,  64,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0, 128, 191, 
      0,   0, 128,  63,   2,  64, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
    128,  63,   0,   0,   0,   0, 
     54,   0,   0,   8,  50,   0, 
     16,   0,   1,   0,   0,   0, 
      2,  64,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,  48,   0,   0,   1, 
     33,   0,   0,   7,  66,   0, 
     16,   0,   1,   0,   0,   0, 
     26,   0,  16,   0,   1,   0, 
      0,   0,   1,  64,   0,   0, 
     66,   0,   0,   0,   3,   0, 
      4,   3,  42,   0,  16,   0, 
      1,   0,   0,   0,  43,   0, 
      0,   5,  66,   0,  16,   0, 
      1,   0,   0,   0,  26,   0, 
     16,   0,   1,   0,   0,   0, 
     56,   0,   0,   7, 130,   0, 
     16,   0,   1,   0,   0,   0, 
     42,   0,  16,   0,   1,   0, 
      0,   0,   1,  64,   0,   0, 
     53, 165, 231,  64,  50,   0, 
      0,  15, 114,   0,  16,   0, 
      2,   0,   0,   0, 166,  10, 
     16,   0,   1,   0,   0,   0, 
      2,  64,   0,   0,   0,   0, 
      0,  63, 143, 194, 117,  60, 
     10, 215,  35,  60,   0,   0, 
      0,   0,   2,  64,   0,   0, 
      0,   0, 128,  63,   0,   0, 
    128,  63,   0,   0, 128,  63, 
      0,   0,   0,   0,  56,   0, 
      0,   7, 130,   0,  16,   0, 
      2,   0,   0,   0,  42,   0, 
     16,   0,   0,   0,   0,   0, 
     10,   0,  16,   0,   2,   0, 
      0,   0,  65,   0,   0,   5, 
    130,   0,  16,   0,   1,   0, 
      0,   0,  58,   0,  16,   0, 
      1,   0,   0,   0,  50,   0, 
      0,  10, 130,   0,  16,   0, 
      1,   0,   0,   0,  42,   0, 
     16,   0,   1,   0,   0,   0, 
      1,  64,   0,   0,  53, 165, 
    231,  64,  58,   0,  16, 128, 
     65,   0,   0,   0,   1,   0, 
      0,   0,  50,   0,   0,  10, 
     18,   0,  16,   0,   3,   0, 
      0,   0,  42, 128,  32,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   1,  64,   0,   0, 
      0,   0,   0,  64,  42,   0, 
     16,   0,   1,   0,   0,   0, 
     77,   0,   0,   6,  18,   0, 
     16,   0,   3,   0,   0,   0, 
      0, 208,   0,   0,  10,   0, 
     16,   0,   3,   0,   0,   0, 
     50,   0,   0,  10, 130,   0, 
     16,   0,   1,   0,   0,   0, 
     10,   0,  16, 128,  65,   0, 
      0,   0,   3,   0,   0,   0, 
      1,  64,   0,   0, 205, 204, 
    204,  61,  58,   0,  16,   0, 
      1,   0,   0,   0,  56,   0, 
      0,   7,  34,   0,  16,   0, 
      3,   0,   0,   0,  58,   0, 
     16,   0,   1,   0,   0,   0, 
     58,   0,  16,   0,   2,   0, 
      0,   0,  14,   0,   0,   7, 
     18,   0,  16,   0,   3,   0, 
      0,   0,  26,   0,  16,   0, 
      0,   0,   0,   0,  26,   0, 
     16,   0,   2,   0,   0,   0, 
     50,   0,   0,   9,  50,   0, 
     16,   0,   2,   0,   0,   0, 
    230,  10,  16,   0,   0,   0, 
      0,   0,   6,   0,  16,   0, 
      2,   0,   0,   0,  70,   0, 
     16,   0,   3,   0,   0,   0, 
     65,   0,   0,   5,  50,   0, 
     16,   0,   3,   0,   0,   0, 
     22,   5,  16,   0,   2,   0, 
      0,   0,   0,   0,   0,   7, 
    130,   0,  16,   0,   1,   0, 
      0,   0,  42,   0,  16,   0, 
      1,   0,   0,   0,   1,  64, 
      0,   0,  18, 131, 249,  65, 
     65,   0,   0,   5,  66,   0, 
     16,   0,   3,   0,   0,   0, 
     58,   0,  16,   0,   1,   0, 
      0,   0,  50,   0,   0,  15, 
    114,   0,  16,   0,   4,   0, 
      0,   0,  70,   2,  16,   0, 
      3,   0,   0,   0,   2,  64, 
      0,   0, 172, 197,  39,  55, 
    172, 197,  39,  55, 172, 197, 
     39,  55,   0,   0,   0,   0, 
      2,  64,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0, 137,  65,  62,   0,   0, 
      0,   0,  50,   0,   0,  15, 
    194,   0,  16,   0,   3,   0, 
      0,   0,   6,   4,  16,   0, 
      3,   0,   0,   0,   2,  64, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0, 172, 197, 
     39,  55, 172, 197,  39,  55, 
      2,  64,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
    205, 111, 245,  70, 205, 111, 
    245,  70,  16,   0,   0,  10, 
    130,   0,  16,   0,   1,   0, 
      0,   0,   2,  64,   0,   0, 
    130,  43,  85,  65, 240,  22, 
    188,  65, 153, 176, 173,  65, 
      0,   0,   0,   0,  70,   2, 
     16,   0,   4,   0,   0,   0, 
     16,   0,   0,  10, 130,   0, 
     16,   0,   2,   0,   0,   0, 
      2,  64,   0,   0,  56, 248, 
    168,  65, 127, 217, 229,  65, 
     50, 230,  62,  65,   0,   0, 
      0,   0,  70,   2,  16,   0, 
      4,   0,   0,   0,  26,   0, 
      0,   5,  18,   0,  16,   0, 
      4,   0,   0,   0,  58,   0, 
     16,   0,   1,   0,   0,   0, 
     26,   0,   0,   5,  34,   0, 
     16,   0,   4,   0,   0,   0, 
     58,   0,  16,   0,   2,   0, 
      0,   0,  14,   0,   0,   7, 
    194,   0,  16,   0,   3,   0, 
      0,   0, 166,  14,  16,   0, 
      3,   0,   0,   0,   6,   4, 
     16,   0,   4,   0,   0,   0, 
     26,   0,   0,   5, 194,   0, 
     16,   0,   3,   0,   0,   0, 
    166,  14,  16,   0,   3,   0, 
      0,   0,   0,   0,   0,   8, 
     50,   0,  16,   0,   3,   0, 
      0,   0,  22,   5,  16,   0, 
      2,   0,   0,   0,  70,   0, 
     16, 128,  65,   0,   0,   0, 
      3,   0,   0,   0,   0,   0, 
      0,  10,  50,   0,  16,   0, 
      3,   0,   0,   0,  70,   0, 
     16,   0,   3,   0,   0,   0, 
      2,  64,   0,   0,   0,   0, 
      0, 191,   0,   0,   0, 191, 
      0,   0,   0,   0,   0,   0, 
      0,   0,  50,   0,   0,  12, 
     50,   0,  16,   0,   3,   0, 
      0,   0, 230,  10,  16,   0, 
      3,   0,   0,   0,   2,  64, 
      0,   0, 102, 102, 102,  63, 
    102, 102, 102,  63,   0,   0, 
      0,   0,   0,   0,   0,   0, 
     70,   0,  16,   0,   3,   0, 
      0,   0,   0,   0,   0,  10, 
     50,   0,  16,   0,   3,   0, 
      0,   0,  70,   0,  16,   0, 
      3,   0,   0,   0,   2,  64, 
      0,   0, 102, 102, 230, 190, 
    102, 102, 230, 190,   0,   0, 
      0,   0,   0,   0,   0,   0, 
     56,   0,   0,  10,  50,   0, 
     16,   0,   2,   0,   0,   0, 
     70,   0,  16,   0,   2,   0, 
      0,   0,   2,  64,   0,   0, 
      0,   0,  32,  65,   0,   0, 
     32,  65,   0,   0,   0,   0, 
      0,   0,   0,   0,  26,   0, 
      0,   5,  50,   0,  16,   0, 
      2,   0,   0,   0,  70,   0, 
     16,   0,   2,   0,   0,   0, 
     50,   0,   0,  15,  50,   0, 
     16,   0,   2,   0,   0,   0, 
     70,   0,  16,   0,   2,   0, 
      0,   0,   2,  64,   0,   0, 
      0,   0,   0,  64,   0,   0, 
      0,  64,   0,   0,   0,   0, 
      0,   0,   0,   0,   2,  64, 
      0,   0,   0,   0, 128, 191, 
      0,   0, 128, 191,   0,   0, 
      0,   0,   0,   0,   0,   0, 
     50,   0,   0,  14,  50,   0, 
     16,   0,   2,   0,   0,   0, 
     70,   0,  16, 128, 129,   0, 
      0,   0,   2,   0,   0,   0, 
      2,  64,   0,   0,  10, 215, 
     35,  60,  10, 215,  35,  60, 
      0,   0,   0,   0,   0,   0, 
      0,   0,  70,   0,  16, 128, 
    129,   0,   0,   0,   3,   0, 
      0,   0,   0,   0,   0,   8, 
    130,   0,  16,   0,   1,   0, 
      0,   0,  26,   0,  16, 128, 
     65,   0,   0,   0,   2,   0, 
      0,   0,  10,   0,  16,   0, 
      2,   0,   0,   0,   0,   0, 
      0,   7, 130,   0,  16,   0, 
      2,   0,   0,   0,  26,   0, 
     16,   0,   2,   0,   0,   0, 
     10,   0,  16,   0,   2,   0, 
      0,   0,  52,   0,   0,   7, 
    130,   0,  16,   0,   1,   0, 
      0,   0,  58,   0,  16,   0, 
      1,   0,   0,   0,  58,   0, 
     16,   0,   2,   0,   0,   0, 
     52,   0,   0,   7,  18,   0, 
     16,   0,   2,   0,   0,   0, 
     26,   0,  16,   0,   2,   0, 
      0,   0,  10,   0,  16,   0, 
      2,   0,   0,   0,  50,   0, 
      0,   9, 130,   0,  16,   0, 
      1,   0,   0,   0,  58,   0, 
     16,   0,   1,   0,   0,   0, 
      1,  64,   0,   0, 154, 153, 
     25,  63,  10,   0,  16,   0, 
      2,   0,   0,   0,  50,   0, 
      0,  10,  66,   0,  16,   0, 
      1,   0,   0,   0,  10,   0, 
     16, 128,  65,   0,   0,   0, 
      0,   0,   0,   0,   1,  64, 
      0,   0,   0,   0, 160,  64, 
     42,   0,  16,   0,   1,   0, 
      0,   0,   0,   0,   0,  10, 
    194,   0,  16,   0,   1,   0, 
      0,   0, 166,  14,  16,   0, 
      1,   0,   0,   0,   2,  64, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
    160, 192,  10, 215,  35, 188, 
     56,   0,   0,   8,  66,   0, 
     16,   0,   1,   0,   0,   0, 
     42,   0,  16, 128, 129,   0, 
      0,   0,   1,   0,   0,   0, 
      1,  64,   0,   0,   0,   0, 
      0,  63,  51,   0,   0,   7, 
     66,   0,  16,   0,   1,   0, 
      0,   0,  42,   0,  16,   0, 
      1,   0,   0,   0,   1,  64, 
      0,   0,   0,   0, 128,  63, 
     50,   0,   0,   9,  66,   0, 
     16,   0,   1,   0,   0,   0, 
     42,   0,  16,   0,   1,   0, 
      0,   0,   1,  64,   0,   0, 
    205, 204,  76,  61,   1,  64, 
      0,   0, 205, 204,  76,  61, 
     56,   0,   0,   7,  18,   0, 
     16,   0,   2,   0,   0,   0, 
     42,   0,  16,   0,   1,   0, 
      0,   0,   1,  64,   0,   0, 
      0,   0,   0, 192,   0,   0, 
      0,   8,  66,   0,  16,   0, 
      1,   0,   0,   0,  42,   0, 
     16, 128,  65,   0,   0,   0, 
      1,   0,   0,   0,  58,   0, 
     16,   0,   1,   0,   0,   0, 
     14,   0,   0,  10, 130,   0, 
     16,   0,   1,   0,   0,   0, 
      2,  64,   0,   0,   0,   0, 
    128,  63,   0,   0, 128,  63, 
      0,   0, 128,  63,   0,   0, 
    128,  63,  10,   0,  16,   0, 
      2,   0,   0,   0,  56,  32, 
      0,   7,  66,   0,  16,   0, 
      1,   0,   0,   0,  58,   0, 
     16,   0,   1,   0,   0,   0, 
     42,   0,  16,   0,   1,   0, 
      0,   0,  50,   0,   0,   9, 
    130,   0,  16,   0,   1,   0, 
      0,   0,  42,   0,  16,   0, 
      1,   0,   0,   0,   1,  64, 
      0,   0,   0,   0,   0, 192, 
      1,  64,   0,   0,   0,   0, 
     64,  64,  56,   0,   0,   7, 
     66,   0,  16,   0,   1,   0, 
      0,   0,  42,   0,  16,   0, 
      1,   0,   0,   0,  42,   0, 
     16,   0,   1,   0,   0,   0, 
     56,   0,   0,   7,  66,   0, 
     16,   0,   1,   0,   0,   0, 
     42,   0,  16,   0,   1,   0, 
      0,   0,  58,   0,  16,   0, 
      1,   0,   0,   0,  14,   0, 
      0,   7, 130,   0,  16,   0, 
      1,   0,   0,   0,  42,   0, 
     16,   0,   3,   0,   0,   0, 
     42,   0,  16,   0,   2,   0, 
      0,   0,  50,   0,   0,   9, 
     18,   0,  16,   0,   1,   0, 
      0,   0,  42,   0,  16,   0, 
      1,   0,   0,   0,  58,   0, 
     16,   0,   1,   0,   0,   0, 
     10,   0,  16,   0,   1,   0, 
      0,   0,  30,   0,   0,   7, 
     34,   0,  16,   0,   1,   0, 
      0,   0,  26,   0,  16,   0, 
      1,   0,   0,   0,   1,  64, 
      0,   0,   1,   0,   0,   0, 
     22,   0,   0,   1,  54,   0, 
      0,   5, 130,  32,  16,   0, 
      0,   0,   0,   0,  10,   0, 
     16,   0,   1,   0,   0,   0, 
     54,   0,   0,   8, 114,  32, 
     16,   0,   0,   0,   0,   0, 
      2,  64,   0,   0,   0,   0, 
    128,  63,   0,   0, 128,  63, 
      0,   0, 128,  63,   0,   0, 
      0,   0,  62,   0,   0,   1, 
     83,  84,  65,  84, 116,   0, 
      0,   0,  62,   0,   0,   0, 
      5,   0,   0,   0,   0,   0, 
      0,   0,   2,   0,   0,   0, 
     52,   0,   0,   0,   2,   0, 
      0,   0,   0,   0,   0,   0, 
      1,   0,   0,   0,   1,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      3,   0,   0,   0,   0,   0, 
      0,   0,   8,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0
};

const BYTE g_VertyShader[] = {
	68, 88, 66, 67, 76, 75,
	108, 163, 29, 221, 215, 151,
	233, 28, 62, 114, 145, 31,
	52, 111, 1, 0, 0, 0,
	176, 2, 0, 0, 5, 0,
	0, 0, 52, 0, 0, 0,
	128, 0, 0, 0, 180, 0,
	0, 0, 12, 1, 0, 0,
	52, 2, 0, 0, 82, 68,
	69, 70, 68, 0, 0, 0,
	0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0,
	28, 0, 0, 0, 0, 4,
	254, 255, 0, 1, 0, 0,
	28, 0, 0, 0, 77, 105,
	99, 114, 111, 115, 111, 102,
	116, 32, 40, 82, 41, 32,
	72, 76, 83, 76, 32, 83,
	104, 97, 100, 101, 114, 32,
	67, 111, 109, 112, 105, 108,
	101, 114, 32, 49, 48, 46,
	49, 0, 73, 83, 71, 78,
	44, 0, 0, 0, 1, 0,
	0, 0, 8, 0, 0, 0,
	32, 0, 0, 0, 0, 0,
	0, 0, 6, 0, 0, 0,
	1, 0, 0, 0, 0, 0,
	0, 0, 1, 1, 0, 0,
	83, 86, 95, 86, 69, 82,
	84, 69, 88, 73, 68, 0,
	79, 83, 71, 78, 80, 0,
	0, 0, 2, 0, 0, 0,
	8, 0, 0, 0, 56, 0,
	0, 0, 0, 0, 0, 0,
	1, 0, 0, 0, 3, 0,
	0, 0, 0, 0, 0, 0,
	15, 0, 0, 0, 68, 0,
	0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 3, 0,
	0, 0, 1, 0, 0, 0,
	3, 12, 0, 0, 83, 86,
	95, 80, 79, 83, 73, 84,
	73, 79, 78, 0, 84, 69,
	88, 67, 79, 79, 82, 68,
	0, 171, 171, 171, 83, 72,
	68, 82, 32, 1, 0, 0,
	64, 0, 1, 0, 72, 0,
	0, 0, 96, 0, 0, 4,
	18, 16, 16, 0, 0, 0,
	0, 0, 6, 0, 0, 0,
	103, 0, 0, 4, 242, 32,
	16, 0, 0, 0, 0, 0,
	1, 0, 0, 0, 101, 0,
	0, 3, 50, 32, 16, 0,
	1, 0, 0, 0, 104, 0,
	0, 2, 1, 0, 0, 0,
	54, 0, 0, 8, 194, 32,
	16, 0, 0, 0, 0, 0,
	2, 64, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0,
	128, 63, 1, 0, 0, 7,
	18, 0, 16, 0, 0, 0,
	0, 0, 10, 16, 16, 0,
	0, 0, 0, 0, 1, 64,
	0, 0, 1, 0, 0, 0,
	85, 0, 0, 7, 66, 0,
	16, 0, 0, 0, 0, 0,
	10, 16, 16, 0, 0, 0,
	0, 0, 1, 64, 0, 0,
	1, 0, 0, 0, 86, 0,
	0, 5, 50, 0, 16, 0,
	0, 0, 0, 0, 134, 0,
	16, 0, 0, 0, 0, 0,
	0, 0, 0, 10, 194, 0,
	16, 0, 0, 0, 0, 0,
	6, 4, 16, 0, 0, 0,
	0, 0, 2, 64, 0, 0,
	0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 191,
	0, 0, 0, 191, 54, 0,
	0, 5, 50, 32, 16, 0,
	1, 0, 0, 0, 70, 0,
	16, 0, 0, 0, 0, 0,
	0, 0, 0, 7, 18, 32,
	16, 0, 0, 0, 0, 0,
	42, 0, 16, 0, 0, 0,
	0, 0, 42, 0, 16, 0,
	0, 0, 0, 0, 56, 0,
	0, 7, 34, 32, 16, 0,
	0, 0, 0, 0, 58, 0,
	16, 0, 0, 0, 0, 0,
	1, 64, 0, 0, 0, 0,
	0, 192, 62, 0, 0, 1,
	83, 84, 65, 84, 116, 0,
	0, 0, 9, 0, 0, 0,
	1, 0, 0, 0, 0, 0,
	0, 0, 3, 0, 0, 0,
	3, 0, 0, 0, 0, 0,
	0, 0, 2, 0, 0, 0,
	1, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0,
	2, 0, 0, 0, 0, 0,
	0, 0, 1, 0, 0, 0,
	0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0,
	0, 0, 0, 0
};

#include <dwmapi.h>
#include <mmsystem.h>
#include <setupapi.h>

static const GUID GUID_DEVINTERFACE_DISPLAY_ADAPTER = { 0x5b45201d, 0xf2f2, 0x4f3b, { 0x85, 0xbb, 0x30, 0xff, 0x1f, 0x95, 0x35, 0x99 } };

#pragma comment(lib, "dwmapi")

static bool IsSafeGPUDriver()
{
	static auto hSetupAPI = LoadLibraryW(L"setupapi.dll");
	if (!hSetupAPI)
	{
		return false;
	}

	static auto _SetupDiGetClassDevsW = (decltype(&SetupDiGetClassDevsW))GetProcAddress(hSetupAPI, "SetupDiGetClassDevsW");
	static auto _SetupDiBuildDriverInfoList = (decltype(&SetupDiBuildDriverInfoList))GetProcAddress(hSetupAPI, "SetupDiBuildDriverInfoList");
	static auto _SetupDiEnumDeviceInfo = (decltype(&SetupDiEnumDeviceInfo))GetProcAddress(hSetupAPI, "SetupDiEnumDeviceInfo");
	static auto _SetupDiEnumDriverInfoW = (decltype(&SetupDiEnumDriverInfoW))GetProcAddress(hSetupAPI, "SetupDiEnumDriverInfoW");
	static auto _SetupDiDestroyDeviceInfoList = (decltype(&SetupDiDestroyDeviceInfoList))GetProcAddress(hSetupAPI, "SetupDiDestroyDeviceInfoList");

	HDEVINFO devInfoSet = _SetupDiGetClassDevsW(&GUID_DEVINTERFACE_DISPLAY_ADAPTER, NULL, NULL,
	DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

	bool safe = true;

	for (int i = 0;; i++)
	{
		SP_DEVINFO_DATA devInfo = { sizeof(SP_DEVINFO_DATA) };
		if (!_SetupDiEnumDeviceInfo(devInfoSet, i, &devInfo))
		{
			break;
		}

		if (!_SetupDiBuildDriverInfoList(devInfoSet, &devInfo, SPDIT_COMPATDRIVER))
		{
			safe = false;
			break;
		}

		SP_DRVINFO_DATA drvInfo = { sizeof(SP_DRVINFO_DATA) };
		if (_SetupDiEnumDriverInfoW(devInfoSet, &devInfo, SPDIT_COMPATDRIVER, 0, &drvInfo))
		{
			ULARGE_INTEGER driverDate = {0};
			driverDate.HighPart = drvInfo.DriverDate.dwHighDateTime;
			driverDate.LowPart = drvInfo.DriverDate.dwLowDateTime;
			
			// drivers from after 2007-01-01 (to prevent in-box driver from being wrong) and 2020-01-01 are 'unsafe' and might crash
			if (driverDate.QuadPart >= 128120832000000000ULL && driverDate.QuadPart < 132223104000000000ULL)
			{
				safe = false;
				break;
			}
		}
	}

	_SetupDiDestroyDeviceInfoList(devInfoSet);

	return safe;
}

static void InitializeRenderOverlay(winrt::Windows::UI::Xaml::Controls::SwapChainPanel swapChainPanel, int w, int h)
{
	auto nativePanel = swapChainPanel.as<ISwapChainPanelNative>();

	auto run = [w, h, swapChainPanel, nativePanel]()
	{
		auto loadSystemDll = [](auto dll)
		{
			wchar_t systemPath[512];
			GetSystemDirectory(systemPath, _countof(systemPath));

			wcscat_s(systemPath, dll);

			return LoadLibrary(systemPath);
		};

		ComPtr<ID3D11Device> g_pd3dDevice = NULL;
		ComPtr<ID3D11DeviceContext> g_pd3dDeviceContext = NULL;
		ComPtr<IDXGISwapChain1> g_pSwapChain = NULL;
		ComPtr<ID3D11RenderTargetView> g_mainRenderTargetView = NULL;

		// Setup swap chain
		DXGI_SWAP_CHAIN_DESC1 sd;
		ZeroMemory(&sd, sizeof(sd));
		sd.BufferCount = 2;
		sd.Width = w;
		sd.Height = h;
		sd.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		sd.AlphaMode = DXGI_ALPHA_MODE_PREMULTIPLIED;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
		sd.Scaling = DXGI_SCALING_STRETCH;
		sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;

		UINT createDeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
		//createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
		D3D_FEATURE_LEVEL featureLevel;

		// so we'll fail if FL11 isn't supported
		const D3D_FEATURE_LEVEL featureLevelArray[1] = {
			D3D_FEATURE_LEVEL_11_0,
		};

		auto d3d11 = loadSystemDll(L"\\d3d11.dll");
		auto _D3D11CreateDevice = (decltype(&D3D11CreateDevice))GetProcAddress(d3d11, "D3D11CreateDevice");

		if (_D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags, featureLevelArray, std::size(featureLevelArray), D3D11_SDK_VERSION, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext) != S_OK)
		{
			return;
		}

		ComPtr<IDXGIDevice1> device1;
		if (FAILED(g_pd3dDevice.As(&device1)))
		{
			return;
		}

		ComPtr<IDXGIAdapter> adapter;
		if (FAILED(device1->GetAdapter(&adapter)))
		{
			return;
		}

		ComPtr<IDXGIFactory> parent;
		if (FAILED(adapter->GetParent(__uuidof(IDXGIFactory), &parent)))
		{
			return;
		}

		ComPtr<IDXGIFactory3> factory3;
		if (FAILED(parent.As(&factory3)))
		{
			return;
		}

		if (FAILED(factory3->CreateSwapChainForComposition(g_pd3dDevice.Get(), &sd, NULL, &g_pSwapChain)))
		{
			return;
		}

		{
			ComPtr<ID3D11Texture2D> pBackBuffer;
			g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
			g_pd3dDevice->CreateRenderTargetView(pBackBuffer.Get(), NULL, &g_mainRenderTargetView);
		}

		swapChainPanel.Dispatcher().TryRunAsync(
		winrt::Windows::UI::Core::CoreDispatcherPriority::Normal,
		[g_pSwapChain, nativePanel]()
		{
			nativePanel->SetSwapChain(g_pSwapChain.Get());
		});

		ComPtr<ID3D11VertexShader> vs;
		ComPtr<ID3D11PixelShader> ps;

		g_pd3dDevice->CreatePixelShader(g_PixyShader, sizeof(g_PixyShader), NULL, &ps);
		g_pd3dDevice->CreateVertexShader(g_VertyShader, sizeof(g_VertyShader), NULL, &vs);

		ComPtr<ID3D11BlendState> bs;

		{
			D3D11_BLEND_DESC desc = { 0 };
			desc.AlphaToCoverageEnable = false;
			desc.RenderTarget[0].BlendEnable = true;
			desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
			desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
			desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
			desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
			desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
			desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
			desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

			g_pd3dDevice->CreateBlendState(&desc, &bs);
		}

		struct CBuf
		{
			float res[2];
			float sec;
			float pad;
		};

		ComPtr<ID3D11Buffer> cbuf;

		{
			D3D11_BUFFER_DESC desc;
			desc.ByteWidth = sizeof(CBuf);
			desc.Usage = D3D11_USAGE_DYNAMIC;
			desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			desc.MiscFlags = 0;
			g_pd3dDevice->CreateBuffer(&desc, NULL, &cbuf);
		}

		while (g_uui.ten)
		{
			// Setup viewport
			D3D11_VIEWPORT vp;
			memset(&vp, 0, sizeof(D3D11_VIEWPORT));
			vp.Width = w;
			vp.Height = h;
			vp.MinDepth = 0.0f;
			vp.MaxDepth = 1.0f;
			vp.TopLeftX = vp.TopLeftY = 0;
			g_pd3dDeviceContext->RSSetViewports(1, &vp);

			auto rtv = g_mainRenderTargetView.Get();
			g_pd3dDeviceContext->OMSetRenderTargets(1, &rtv, NULL);

			float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
			g_pd3dDeviceContext->ClearRenderTargetView(rtv, clearColor);

			g_pd3dDeviceContext->VSSetShader(vs.Get(), NULL, 0);
			g_pd3dDeviceContext->PSSetShader(ps.Get(), NULL, 0);

			auto cb = cbuf.Get();
			g_pd3dDeviceContext->VSSetConstantBuffers(0, 1, &cb);
			g_pd3dDeviceContext->PSSetConstantBuffers(0, 1, &cb);

			g_pd3dDeviceContext->OMSetBlendState(bs.Get(), NULL, 0xFFFFFFFF);

			static auto startTime = timeGetTime();

			D3D11_MAPPED_SUBRESOURCE mapped_resource;
			if (SUCCEEDED(g_pd3dDeviceContext->Map(cb, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_resource)))
			{
				auto c = (CBuf*)mapped_resource.pData;
				c->res[0] = float(w);
				c->res[1] = float(h);
				c->sec = (timeGetTime() - startTime) / 1000.0f;
				g_pd3dDeviceContext->Unmap(cb, 0);
			}

			g_pd3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
			g_pd3dDeviceContext->Draw(4, 0);

			g_pSwapChain->Present(0, 0);
			DwmFlush();
		}
	};

	std::thread([run]()
	{
		if (IsSafeGPUDriver())
		{
			run();
		}

		// prevent the thread from exiting (the CRT is broken and will crash on thread exit in some cases)
		WaitForSingleObject(GetCurrentProcess(), INFINITE);
	}).detach();
}

void UI_CreateWindow()
{
	g_uui.taskbarMsg = RegisterWindowMessage(L"TaskbarButtonCreated");

	HWND rootWindow = CreateWindowEx(0, L"NotSteamAtAll", PRODUCT_NAME, 13238272 /* lol */, 0x80000000, 0, g_dpi.ScaleX(500), g_dpi.ScaleY(129), NULL, NULL, GetModuleHandle(NULL), 0);

	int wwidth = 500;
	int wheight = 139;

	if (!g_uui.tenMode)
	{
		INITCOMMONCONTROLSEX controlSex;
		controlSex.dwSize = sizeof(controlSex);
		controlSex.dwICC = 16416; // lazy bum
		InitCommonControlsEx(&controlSex);

		HFONT font = UI_CreateScaledFont(-12, 0, 0, 0, 0, 0, 0, 0, 1, 8, 0, 5, 2, L"Tahoma");

		// TODO: figure out which static is placed where
		HWND static1 = CreateWindowEx(0x20, L"static", L"static1", 0x50000000, g_dpi.ScaleX(15), g_dpi.ScaleY(15), g_dpi.ScaleX(455), g_dpi.ScaleY(25), rootWindow, 0, GetModuleHandle(NULL) /* what?! */, 0);

		SendMessage(static1, WM_SETFONT, (WPARAM)font, 0);

		HWND cancelButton = CreateWindowEx(0, L"button", L"Cancel", 0x50000000, g_dpi.ScaleX(395), g_dpi.ScaleY(64), g_dpi.ScaleX(75), g_dpi.ScaleY(25), rootWindow, 0, GetModuleHandle(NULL), 0);
		SendMessage(cancelButton, WM_SETFONT, (WPARAM)font, 0);

		HWND progressBar = CreateWindowEx(0, L"msctls_progress32", 0, 0x50000000, g_dpi.ScaleX(15), g_dpi.ScaleY(40), g_dpi.ScaleX(455), g_dpi.ScaleY(15), rootWindow, 0, GetModuleHandle(NULL), 0);
		SendMessage(progressBar, PBM_SETRANGE32, 0, 10000);

		HWND static2 = CreateWindowEx(0x20, L"static", L"static2", 0x50000000, g_dpi.ScaleX(15), g_dpi.ScaleY(68), g_dpi.ScaleX(370), g_dpi.ScaleY(25), rootWindow, 0, GetModuleHandle(NULL) /* what?! */, 0);
		SendMessage(static2, WM_SETFONT, (WPARAM)font, 0);

		g_uui.cancelButton = cancelButton;
		g_uui.progressBar = progressBar;
		g_uui.topStatic = static1;
		g_uui.bottomStatic = static2;
	}
	else
	{
		wwidth = 525;
		wheight = 525;

		// make TenUI
		auto ten = std::make_unique<TenUI>();
		ten->uiSource = std::move(DesktopWindowXamlSource{});

		// attach window
		auto interop = ten->uiSource.as<IDesktopWindowXamlSourceNative>();
		winrt::check_hresult(interop->AttachToWindow(rootWindow));

		// setup position
		HWND childHwnd;
		interop->get_WindowHandle(&childHwnd);

		SetWindowLong(childHwnd, GWL_EXSTYLE, GetWindowLong(childHwnd, GWL_EXSTYLE) | WS_EX_TRANSPARENT | WS_EX_LAYERED);

		SetWindowPos(childHwnd, 0, 0, 0, g_dpi.ScaleX(wwidth), g_dpi.ScaleY(wheight), SWP_SHOWWINDOW);

		auto doc = winrt::Windows::UI::Xaml::Markup::XamlReader::Load(g_mainXaml);
		auto ui = doc.as<winrt::Windows::UI::Xaml::FrameworkElement>();

		auto bg = ui.FindName(L"BackdropGrid").as<winrt::Windows::UI::Xaml::Controls::Grid>();
		bg.Background(winrt::make<BackdropBrush>());

		{
			auto sc = ui.FindName(L"Overlay").as<winrt::Windows::UI::Xaml::Controls::SwapChainPanel>();

			if (_time64(NULL) < 1675206000)
			{
				InitializeRenderOverlay(sc, g_dpi.ScaleX(wwidth), g_dpi.ScaleY(wheight));
			}
		}

		/*auto shadow = ui.FindName(L"SharedShadow").as<winrt::Windows::UI::Xaml::Media::ThemeShadow>();
		shadow.Receivers().Append(bg);*/

		ten->topStatic = ui.FindName(L"static1").as<winrt::Windows::UI::Xaml::Controls::TextBlock>();
		ten->bottomStatic = ui.FindName(L"static2").as<winrt::Windows::UI::Xaml::Controls::TextBlock>();
		ten->progressBar = ui.FindName(L"progressBar").as<winrt::Windows::UI::Xaml::Controls::ProgressBar>();
		ten->snailContainer = ui.FindName(L"snailContainer").as<winrt::Windows::UI::Xaml::UIElement>();

		ten->uiSource.Content(ui);

		g_uui.tenWindow = FindWindowExW(rootWindow, NULL, L"Windows.UI.Core.CoreWindow", NULL);

		g_uui.ten = std::move(ten);
	}

	g_uui.rootWindow = rootWindow;

	RECT wndRect;
	wndRect.left = 0;
	wndRect.top = 0;
	wndRect.right = g_dpi.ScaleX(wwidth);
	wndRect.bottom = g_dpi.ScaleY(wheight);

	HWND desktop = GetDesktopWindow();
	HDC dc = GetDC(desktop);
	int width = GetDeviceCaps(dc, 8);
	int height = GetDeviceCaps(dc, 10);

	ReleaseDC(desktop, dc);

	SetTimer(rootWindow, 0, 20, NULL);

	MoveWindow(rootWindow, (width - g_dpi.ScaleX(wwidth)) / 2, (height - g_dpi.ScaleY(wheight)) / 2, wndRect.right - wndRect.left, wndRect.bottom - wndRect.top, TRUE);

	ShowWindow(rootWindow, TRUE);
}

LRESULT CALLBACK UI_WndProc(HWND hWnd, UINT uMsg, WPARAM wparam, LPARAM lparam)
{
	switch (uMsg)
	{
		case WM_NCHITTEST:
			if (g_uui.tenMode)
			{
				return HTCAPTION;
			}
		case WM_NCCALCSIZE:
			if (g_uui.tenMode)
			{
				return 0;
			}
		case WM_NCCREATE:
			{
				// Only Windows 10+ supports EnableNonClientDpiScaling
				if (IsWindows10OrGreater())
				{
					HMODULE user32 = LoadLibrary(L"user32.dll");

					if (user32)
					{
						auto EnableNonClientDpiScaling = (decltype(&::EnableNonClientDpiScaling))GetProcAddress(user32, "EnableNonClientDpiScaling");

						if (EnableNonClientDpiScaling)
						{
							EnableNonClientDpiScaling(hWnd);
						}

						FreeLibrary(user32);
					}
				}

				return DefWindowProc(hWnd, uMsg, wparam, lparam);
			}
		
		case WM_CTLCOLORSTATIC:
			SetBkMode((HDC)wparam, TRANSPARENT);
			SetTextColor((HDC)wparam, COLORREF(GetSysColor(COLOR_WINDOWTEXT)));

			return (LRESULT)GetSysColorBrush(COLOR_WINDOW);
		case WM_COMMAND:
			if ((HWND)lparam == g_uui.cancelButton)
			{
				g_uui.canceled = true;
			}

			break;
		case WM_TIMER:
			SetWindowText(g_uui.topStatic, g_uui.topText);
			SetWindowText(g_uui.bottomStatic, g_uui.bottomText);
			break;
		case WM_PAINT:
			{
				PAINTSTRUCT ps;
				HDC dc = BeginPaint(hWnd, &ps);
			
				EndPaint(hWnd, &ps);
				break;
			}
		case WM_DPICHANGED:
			{
				// Set the new DPI
				g_dpi.SetScale(LOWORD(wparam), HIWORD(wparam));

				// Resize the window
				LPRECT newScale = (LPRECT)lparam;
				SetWindowPos(hWnd, HWND_TOP, newScale->left, newScale->top, newScale->right - newScale->left, newScale->bottom - newScale->top, SWP_NOZORDER | SWP_NOACTIVATE);

				// Recreate the font
				HFONT newFont = UI_CreateScaledFont(-12, 0, 0, 0, 0, 0, 0, 0, 1, 8, 0, 5, 2, L"Tahoma");

				// Resize all components
				SetWindowPos(g_uui.topStatic, HWND_TOP, g_dpi.ScaleX(15), g_dpi.ScaleY(15), g_dpi.ScaleX(455), g_dpi.ScaleY(25), SWP_SHOWWINDOW);
				SendMessage(g_uui.topStatic, WM_SETFONT, (WPARAM)newFont, 0);

				SetWindowPos(g_uui.cancelButton, HWND_TOP, g_dpi.ScaleX(395), g_dpi.ScaleY(64), g_dpi.ScaleX(75), g_dpi.ScaleY(25), SWP_SHOWWINDOW);
				SendMessage(g_uui.cancelButton, WM_SETFONT, (WPARAM)newFont, 0);

				SetWindowPos(g_uui.progressBar, HWND_TOP, g_dpi.ScaleX(15), g_dpi.ScaleY(40), g_dpi.ScaleX(455), g_dpi.ScaleY(15), SWP_SHOWWINDOW);

				SetWindowPos(g_uui.bottomStatic, HWND_TOP, g_dpi.ScaleX(15), g_dpi.ScaleY(68), g_dpi.ScaleX(370), g_dpi.ScaleY(25), SWP_SHOWWINDOW);
				SendMessage(g_uui.bottomStatic, WM_SETFONT, (WPARAM)newFont, 0);
				break;
			}
		case WM_CLOSE:
			g_uui.canceled = true;
			return 0;
		default:
			if (uMsg == g_uui.taskbarMsg)
			{
				if (g_uui.tbList)
				{
					g_uui.tbList->SetProgressState(hWnd, TBPF_NORMAL);
					g_uui.tbList->SetProgressValue(hWnd, 0, 100);
				}
			}
			break;
	}

	return DefWindowProc(hWnd, uMsg, wparam, lparam);
}

void UI_RegisterClass()
{
	WNDCLASSEX wndClass = { 0 };
	wndClass.cbSize = sizeof(wndClass);
	wndClass.style = 3;
	wndClass.lpfnWndProc = UI_WndProc;
	wndClass.cbClsExtra = 0;
	wndClass.cbWndExtra = 0;
	wndClass.hInstance = GetModuleHandle(NULL);
	wndClass.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(1));
	wndClass.hCursor = LoadCursor(NULL, (LPCWSTR)0x7F02);
	wndClass.hbrBackground = (HBRUSH)6;
	wndClass.lpszClassName = L"NotSteamAtAll";
	wndClass.hIconSm = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(1));

	RegisterClassEx(&wndClass);
}

struct TenUIStorage;

static TenUIStorage* g_tenUI;

struct TenUIStorage : public TenUIBase
{
	// convoluted stuff to prevent WindowsXamlManager destruction weirdness
	static inline thread_local WindowsXamlManager* gManager{ nullptr };

	TenUIStorage()
	{
		g_tenUI = this;
	}

	void InitManager()
	{
		if (!gManager)
		{
			static thread_local WindowsXamlManager manager = WindowsXamlManager::InitializeForCurrentThread();
			gManager = &manager;
		}
	}

	virtual ~TenUIStorage() override
	{
		ShowWindow(g_uui.tenWindow, SW_HIDE);

		g_tenUI = nullptr;
	}

	static void ReallyBreakIt()
	{
		if (gManager)
		{
			gManager->Close();
		}
	}
};

std::unique_ptr<TenUIBase> UI_InitTen()
{
	// Windows 10 RS5+ gets a neat UI
	DWORDLONG viMask = 0;
	OSVERSIONINFOEXW osvi = { 0 };
	osvi.dwOSVersionInfoSize = sizeof(osvi);
	osvi.dwBuildNumber = 17763; // RS5+

	VER_SET_CONDITION(viMask, VER_BUILDNUMBER, VER_GREATER_EQUAL);

	bool forceOff = false;

	static HostSharedData<CfxState> initState("CfxInitState");

	if (initState->isReverseGame)
	{
		forceOff = true;
	}

	if (getenv("CitizenFX_NoTenUI"))
	{
		forceOff = true;
	}

#ifdef IS_LAUNCHER
	forceOff = true;
#endif

	if (VerifyVersionInfoW(&osvi, VER_BUILDNUMBER, viMask) && !forceOff)
	{
		RO_REGISTRATION_COOKIE cookie;

		g_uui.tenMode = true;

		try
		{
			return std::make_unique<TenUIStorage>();
		}
		catch (const std::exception&)
		{
		}
	}

	return std::make_unique<TenUIBase>();
}

void UI_DestroyTen()
{
	TenUIStorage::ReallyBreakIt();
}

void UI_DoCreation(bool safeMode)
{
	CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

	if (g_tenUI)
	{
		g_tenUI->InitManager();
	}

	if (IsWindows7OrGreater())
	{
		CoCreateInstance(CLSID_TaskbarList, 
			NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&g_uui.tbList));
	}

	// Only Windows 8.1+ supports per-monitor DPI awareness
	if (IsWindows8Point1OrGreater())
	{
		HMODULE shCore = LoadLibrary(L"shcore.dll");

		if (shCore)
		{
			auto GetDpiForMonitor = (decltype(&::GetDpiForMonitor))GetProcAddress(shCore, "GetDpiForMonitor");

			if (GetDpiForMonitor)
			{
				UINT dpiX, dpiY;

				POINT point;
				point.x = 1;
				point.y = 1;

				// Get DPI for the main monitor
				HMONITOR monitor = MonitorFromPoint(point, MONITOR_DEFAULTTONEAREST);
				GetDpiForMonitor(monitor, MDT_EFFECTIVE_DPI, &dpiX, &dpiY);
				g_dpi.SetScale(dpiX, dpiY);
			}

			FreeLibrary(shCore);
		}
	}

	static bool lastTen = g_uui.tenMode;

	if (safeMode)
	{
		lastTen = g_uui.tenMode;
		g_uui.tenMode = false;
	}
	else
	{
		g_uui.tenMode = lastTen;
	}

	UI_RegisterClass();
	UI_CreateWindow();
}

void UI_DoDestruction()
{
	static HostSharedData<CfxState> initState("CfxInitState");
	AllowSetForegroundWindow((initState->gamePid) ? initState->gamePid : GetCurrentProcessId());

	ShowWindow(g_uui.rootWindow, SW_HIDE);

	g_uui.ten = {};

	DestroyWindow(g_uui.rootWindow);
}

void UI_SetSnailState(bool snail)
{
	if (g_uui.ten)
	{
		g_uui.ten->snailContainer.Visibility(snail ? winrt::Windows::UI::Xaml::Visibility::Visible : winrt::Windows::UI::Xaml::Visibility::Collapsed);

		return;
	}
}

void UI_UpdateText(int textControl, const wchar_t* text)
{
	if (g_uui.ten)
	{
		std::wstring tstr = text;

		if (textControl == 0)
		{
			g_uui.ten->topStatic.Text(tstr);
		}
		else
		{
			g_uui.ten->bottomStatic.Text(tstr);
		}

		return;
	}

	if (textControl == 0)
	{
		wcscpy(g_uui.topText, text);
	}
	else
	{
		wcscpy(g_uui.bottomText, text);
	}
}

void UI_UpdateProgress(double percentage)
{
	if (g_uui.ten)
	{
		try
		{
			g_uui.ten->progressBar.Maximum(100.0);
			g_uui.ten->progressBar.Value(percentage);
		}
		catch (...)
		{
		}

		g_uui.ten->progressBar.IsIndeterminate(percentage == 100);

		return;
	}

	SendMessage(g_uui.progressBar, PBM_SETPOS, (int)(percentage * 100), 0);

	if (g_uui.tbList)
	{
		g_uui.tbList->SetProgressValue(g_uui.rootWindow, (int)percentage, 100);

		if (percentage == 100)
		{
			g_uui.tbList->SetProgressState(g_uui.rootWindow, TBPF_NOPROGRESS);
		}
	}
}

bool UI_IsCanceled()
{
	return g_uui.canceled;
}

void UI_DisplayError(const wchar_t* error)
{
	static TASKDIALOGCONFIG taskDialogConfig = { 0 };
	taskDialogConfig.cbSize = sizeof(taskDialogConfig);
	taskDialogConfig.hInstance = GetModuleHandle(nullptr);
	taskDialogConfig.dwFlags = TDF_ENABLE_HYPERLINKS | TDF_SIZE_TO_CONTENT;
	taskDialogConfig.dwCommonButtons = TDCBF_CLOSE_BUTTON;
	taskDialogConfig.pszWindowTitle = L"Error updating " PRODUCT_NAME;
	taskDialogConfig.pszMainIcon = TD_ERROR_ICON;
	taskDialogConfig.pszMainInstruction = NULL;
	taskDialogConfig.pszContent = error;

	TaskDialogIndirect(&taskDialogConfig, nullptr, nullptr, nullptr);
}

#include <wrl/module.h>

extern "C" HRESULT __stdcall DllCanUnloadNow()
{
#ifdef _WRL_MODULE_H_
	if (!::Microsoft::WRL::Module<::Microsoft::WRL::InProc>::GetModule().Terminate())
	{
		return 1; // S_FALSE
	}
#endif

	if (winrt::get_module_lock())
	{
		return 1; // S_FALSE
	}

	winrt::clear_factory_cache();
	return 0; // S_OK
}

extern "C" DLL_EXPORT HRESULT WINAPI DllGetActivationFactory(HSTRING classId, IActivationFactory** factory)
{
	try
	{
		*factory = nullptr;
		uint32_t length{};
		wchar_t const* const buffer = WindowsGetStringRawBuffer(classId, &length);
		std::wstring_view const name{ buffer, length };

		auto requal = [](std::wstring_view const& left, std::wstring_view const& right) noexcept
		{
			return std::equal(left.rbegin(), left.rend(), right.rbegin(), right.rend());
		};

		if (requal(name, L"CitiLaunch.BackdropBrush"))
		{
			*factory = (IActivationFactory*)winrt::detach_abi(winrt::make<BackdropBrush>());
			return 0;
		}

#ifdef _WRL_MODULE_H_
		return ::Microsoft::WRL::Module<::Microsoft::WRL::InProc>::GetModule().GetActivationFactory(static_cast<HSTRING>(classId), reinterpret_cast<::IActivationFactory * *>(factory));
#else
		return winrt::hresult_class_not_available(name).to_abi();
#endif
	}
	catch (...) { return winrt::to_hresult(); }
}
#endif
