const c = @cImport({
    @cInclude("fenster.h");
});

pub const Fenster = struct {
    fenster: c.fenster,
};

pub fn new(comptime w: i32, comptime h: i32, title: [:0]const u8) type {
    var buf: [w * h]u32 = undefined;
    const f = .{
        .width = w,
        .height = h,
        .title = title,
        .buf = buf,
    };
    c.fenster_open(&f);
    return struct {
        const Self = @This();
        pub fn close(self: *Self) void {
            c.fenster_close(&f);
        }
    };
}
