// M103-S2d (Issue 30 GAP-IMG) 参照：Go image 解码/resize/JPEG（avatar.go 语义对拍）
//   gen   (1) 读 ../m61_gfx/scene.png → image.Decode → 双线性等比 512×384 → jpeg q70
//            → go_ref.jpg（px 产物尺寸/体积量级对拍基准）
//        (2) 生成 200×150 渐变 JPEG → go_input.jpg（供 px 验证 JPEG 输入解码）
//   check 读 px_out.jpg → image.Decode 验格式合法 + 尺寸 512×384 + 与 go_ref.jpg
//         体积同量级（0.3×~3×，JPEG 编码器差异允许）
// 依赖：go 标准库（image/png、image/jpeg）；用法：go build -o build/go_img . && ./go_img gen|check
package main

import (
	"fmt"
	"image"
	"image/color"
	"image/jpeg"
	"image/png"
	"os"
	"path/filepath"
)

func fatal(err error) {
	fmt.Fprintln(os.Stderr, "ERR", err)
	os.Exit(1)
}

// 双线性缩放 RGBA（与 px img_scale 同公式：中心对齐双线性）
func resize(img image.Image, dw, dh int) *image.RGBA {
	sb := img.Bounds()
	sw, sh := sb.Dx(), sb.Dy()
	dst := image.NewRGBA(image.Rect(0, 0, dw, dh))
	for y := 0; y < dh; y++ {
		sy := (float64(y)+0.5)*float64(sh)/float64(dh) - 0.5
		if sy < 0 {
			sy = 0
		}
		y0 := int(sy)
		if y0 >= sh-1 {
			y0 = sh - 1
		}
		y1 := y0 + 1
		if y1 >= sh {
			y1 = y0
		}
		wy := sy - float64(y0)
		for x := 0; x < dw; x++ {
			sx := (float64(x)+0.5)*float64(sw)/float64(dw) - 0.5
			if sx < 0 {
				sx = 0
			}
			x0 := int(sx)
			if x0 >= sw-1 {
				x0 = sw - 1
			}
			x1 := x0 + 1
			if x1 >= sw {
				x1 = x0
			}
			wx := sx - float64(x0)
			// 整像素 RGBA 提取 + 双线性
			r0, g0, b0, a0 := img.At(x0, y0).RGBA()
			r1, g1, b1, a1 := img.At(x1, y0).RGBA()
			r2, g2, b2, a2 := img.At(x0, y1).RGBA()
			r3, g3, b3, a3 := img.At(x1, y1).RGBA()
			rr := lerp(lerp(r0, r1, wx), lerp(r2, r3, wx), wy)
			gg := lerp(lerp(g0, g1, wx), lerp(g2, g3, wx), wy)
			bb := lerp(lerp(b0, b1, wx), lerp(b2, b3, wx), wy)
			aa := lerp(lerp(a0, a1, wx), lerp(a2, a3, wx), wy)
			dst.Set(x, y, color.RGBA{uint8(rr >> 8), uint8(gg >> 8), uint8(bb >> 8), uint8(aa >> 8)})
		}
	}
	return dst
}

func lerp(a, b uint32, t float64) uint32 {
	return uint32(float64(a) + (float64(b)-float64(a))*t + 0.5)
}

func gen() {
	os.MkdirAll("artifacts", 0755)
	// (1) scene.png → 512×384 jpeg q70
	in, err := os.Open(filepath.Join("..", "m61_gfx", "scene.png"))
	if err != nil {
		fatal(err)
	}
	img, err := png.Decode(in)
	in.Close()
	if err != nil {
		fatal(err)
	}
	dst := resize(img, 512, 384)
	ref, err := os.Create("artifacts/go_ref.jpg")
	if err != nil {
		fatal(err)
	}
	if err := jpeg.Encode(ref, dst, &jpeg.Options{Quality: 70}); err != nil {
		fatal(err)
	}
	ref.Close()
	st, _ := os.Stat("artifacts/go_ref.jpg")
	fmt.Printf("go_ref.jpg 512x384 %d bytes\n", st.Size())
	// (2) 200×150 渐变 JPEG 输入
	grad := image.NewRGBA(image.Rect(0, 0, 200, 150))
	for y := 0; y < 150; y++ {
		for x := 0; x < 200; x++ {
			grad.Set(x, y, color.RGBA{uint8(x), uint8(y), uint8((x + y) % 256), 255})
		}
	}
	ji, err := os.Create("artifacts/go_input.jpg")
	if err != nil {
		fatal(err)
	}
	if err := jpeg.Encode(ji, grad, &jpeg.Options{Quality: 85}); err != nil {
		fatal(err)
	}
	ji.Close()
	fmt.Println("go gen done")
}

func check() {
	// px 产出 JPEG：格式合法 + 尺寸 + 体积量级对拍
	px, err := os.Open("artifacts/px_out.jpg")
	if err != nil {
		fatal(err)
	}
	pxImg, err := jpeg.Decode(px)
	px.Close()
	if err != nil {
		fatal(fmt.Errorf("px_out.jpg 无法被 Go image.Decode 解析: %v", err))
	}
	b := pxImg.Bounds()
	if b.Dx() != 512 || b.Dy() != 384 {
		fatal(fmt.Errorf("px_out.jpg 尺寸不符: %dx%d (期望 512x384)", b.Dx(), b.Dy()))
	}
	ps, _ := os.Stat("artifacts/px_out.jpg")
	rs, _ := os.Stat("artifacts/go_ref.jpg")
	pr, gr := float64(ps.Size()), float64(rs.Size())
	if pr < 0.3*gr || pr > 3.0*gr {
		fatal(fmt.Errorf("px jpeg 体积 %d 不在 Go 参照 %d 的 0.3x~3x 量级", ps.Size(), rs.Size()))
	}
	fmt.Printf("m103_s2d_img Go 验 px JPEG: 512x384 合法, px=%dB go_ref=%dB 同量级\n", ps.Size(), rs.Size())
}

func main() {
	os.MkdirAll("artifacts", 0755)
	if len(os.Args) < 2 {
		fmt.Fprintln(os.Stderr, "usage: go_img gen|check")
		os.Exit(2)
	}
	switch os.Args[1] {
	case "gen":
		gen()
	case "check":
		check()
	default:
		fmt.Fprintln(os.Stderr, "usage: go_img gen|check")
		os.Exit(2)
	}
}
