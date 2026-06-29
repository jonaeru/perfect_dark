#!/usr/bin/env swift
// Generates a simple grid/map-themed 1024×1024 PNG for the map editor .app icon.

import AppKit
import Foundation

guard CommandLine.arguments.count >= 2 else {
	fputs("Usage: generate-map-editor-icon.swift <output-png>\n", stderr)
	exit(1)
}

let outputURL = URL(fileURLWithPath: CommandLine.arguments[1])
let size = NSSize(width: 1024, height: 1024)

guard let rep = NSBitmapImageRep(
	bitmapDataPlanes: nil,
	pixelsWide: Int(size.width),
	pixelsHigh: Int(size.height),
	bitsPerSample: 8,
	samplesPerPixel: 4,
	hasAlpha: true,
	isPlanar: false,
	colorSpaceName: .deviceRGB,
	bytesPerRow: 0,
	bitsPerPixel: 0
), let context = NSGraphicsContext(bitmapImageRep: rep) else {
	fputs("Failed to create bitmap context.\n", stderr)
	exit(1)
}

func rgba(_ r: CGFloat, _ g: CGFloat, _ b: CGFloat, _ a: CGFloat = 1) -> NSColor {
	NSColor(calibratedRed: r / 255, green: g / 255, blue: b / 255, alpha: a)
}

let canvas = NSRect(origin: .zero, size: size)
let inset = canvas.insetBy(dx: 48, dy: 48)
let badge = NSBezierPath(roundedRect: inset, xRadius: 200, yRadius: 200)

NSGraphicsContext.saveGraphicsState()
NSGraphicsContext.current = context

rgba(12, 16, 24).setFill()
canvas.fill()

rgba(18, 24, 36).setFill()
badge.fill()

// Grid lines (map editor motif)
context.cgContext.saveGState()
badge.addClip()
rgba(70, 130, 255, 0.35).setStroke()
for i in 0...8 {
	let t = CGFloat(i) / 8.0
	let x = inset.minX + inset.width * t
	let y = inset.minY + inset.height * t
	let vLine = NSBezierPath()
	vLine.move(to: NSPoint(x: x, y: inset.minY))
	vLine.line(to: NSPoint(x: x, y: inset.maxY))
	vLine.lineWidth = i == 4 ? 3 : 1.5
	vLine.stroke()
	let hLine = NSBezierPath()
	hLine.move(to: NSPoint(x: inset.minX, y: y))
	hLine.line(to: NSPoint(x: inset.maxX, y: y))
	hLine.lineWidth = i == 4 ? 3 : 1.5
	hLine.stroke()
}
context.cgContext.restoreGState()

// Pad markers
let padColor = rgba(87, 217, 119, 0.95)
for (cx, cy) in [(0.25, 0.3), (0.72, 0.28), (0.35, 0.68), (0.78, 0.72), (0.5, 0.5)] {
	let px = inset.minX + inset.width * cx
	let py = inset.minY + inset.height * cy
	let dot = NSBezierPath(ovalIn: NSRect(x: px - 22, y: py - 22, width: 44, height: 44))
	padColor.setFill()
	dot.fill()
}

// Title bar strip
let titleRect = NSRect(x: inset.minX + 40, y: inset.maxY - 120, width: inset.width - 80, height: 56)
let titlePath = NSBezierPath(roundedRect: titleRect, xRadius: 14, yRadius: 14)
rgba(255, 255, 255, 0.12).setFill()
titlePath.fill()
rgba(255, 255, 255, 0.85).setFill()
let attrs: [NSAttributedString.Key: Any] = [
	.font: NSFont.systemFont(ofSize: 28, weight: .semibold),
]
let label = "MAP EDITOR" as NSString
let textSize = label.size(withAttributes: attrs)
label.draw(
	at: NSPoint(x: titleRect.midX - textSize.width / 2, y: titleRect.midY - textSize.height / 2),
	withAttributes: attrs
)

let outer = NSBezierPath(roundedRect: inset, xRadius: 200, yRadius: 200)
outer.lineWidth = 8
rgba(255, 255, 255, 0.14).setStroke()
outer.stroke()

NSGraphicsContext.restoreGraphicsState()

guard let pngData = rep.representation(using: .png, properties: [:]) else {
	fputs("Failed to encode PNG.\n", stderr)
	exit(1)
}

do {
	try FileManager.default.createDirectory(at: outputURL.deletingLastPathComponent(), withIntermediateDirectories: true)
	try pngData.write(to: outputURL)
} catch {
	fputs("Failed to write icon: \(error)\n", stderr)
	exit(1)
}
