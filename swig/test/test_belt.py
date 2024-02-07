import bee2py

# begin
buf = bee2py.memAlloc(128)
key = bee2py.new_u32arr(8)


# test keep functions
print(bee2py.beltHash_keep())

#hexFrom test (B194BAC80A08F53B366D008E584A5DE4)
sbuf = "0"*100
sbuf = bee2py.hexFrom(sbuf, bee2py.beltH(), 16)
print(sbuf)

#hexTo test
bee2py.hexTo(buf, "69CCA1C93557C9E3D66BC3E0FA88FA6E")
sbuf = bee2py.hexFrom(sbuf, buf, 16)
print(sbuf)

# belt-block: тест A.1-1
bee2py.memCopy(buf, bee2py.beltH(), 16)
bee2py.beltKeyExpand2(key, bee2py.vp2op(bee2py.ptradd(bee2py.beltH(), 128)), 32)
bee2py.beltBlockEncr(bee2py.vp2op(buf), key);
sbuf = bee2py.hexFrom(sbuf, buf, 16)
print(sbuf)
if sbuf != "69CCA1C93557C9E3D66BC3E0FA88FA6E":
    print("Test A.1-1 failed")
bee2py.beltBlockDecr(bee2py.vp2op(buf), key);
sbuf = bee2py.hexFrom(sbuf, buf, 16)
print(sbuf)
if sbuf != "B194BAC80A08F53B366D008E584A5DE4":
    print("Test A.1-1 failed")

# end
bee2py.memFree(buf)
bee2py.delete_u32arr(key)