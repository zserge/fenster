const std = @import("std");
const c = @cImport({
    @cInclude("fenster.h");
});

pub fn main() void {
    var buf: [320 * 240]u32 = undefined;
    var f = std.mem.zeroInit(c.fenster, .{
        .width = 320,
        .height = 240,
        .title = "hello",
        .buf = &buf[0],
    });
    _ = c.fenster_open(&f);
    defer c.fenster_close(&f);
    var t: u32 = 0;
    var now: i64 = c.fenster_time();
    while (c.fenster_loop(&f) == 0) {
        // Exit when Escape is pressed
        if (f.keys[27] != 0) {
            break;
        }
        // Render x^y^t pattern
        for (buf) |_, i| {
            buf[i] = @intCast(u32, i % 320) ^ @intCast(u32, i / 240) ^ t;
        }
        t +%= 1;
        // Keep ~60 FPS
        var diff: i64 = 1000 / 60 - (c.fenster_time() - now);
        if (diff > 0) {
            c.fenster_sleep(diff);
        }
        now = c.fenster_time();
    }
}
