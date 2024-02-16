package main

import "by.bsu/bee2"
import "fmt"

func main() {
    a := bee2.BeltHash_keep()
    fmt.Println(a)
	cbuf := bee2.MemAlloc(512)

	params := bee2.NewBign_params()
	bee2.BignParamsStd(params,"1.2.112.0.2.0.34.101.45.3.1")
	fmt.Println(params.GetL())
	bee2.HexFrom(bee2.Vp2bp(cbuf), bee2.Op2vp(params.GetQ()), 32)
	sbuf := bee2.Cdata(cbuf, 64)
	fmt.Println(string(sbuf))

	privkey := bee2.MemAlloc(64)
	pubkey := bee2.MemAlloc(128)
	bee2.HexTo(privkey, "1F66B5B84B7339674533F0329C74F21834281FED0732429E0C79235FC273E269")
	bee2.BignPubkeyCalc(bee2.Vp2op(pubkey), params, bee2.Vp2op(privkey))
	bee2.HexFrom(bee2.Vp2bp(cbuf), pubkey, 64)
	sbuf = bee2.Cdata(cbuf, 64)
	fmt.Println(string(sbuf))
	
	hash1 := bee2.MemAlloc(64)
	bee2.BeltHash(bee2.Vp2op(hash1), bee2.Op2vp(bee2.BeltH()), 13)

	count := bee2.New_sizeTarr(1)
	bee2.SizeTarr_setitem(count, 0, 512)
	der := bee2.MemAlloc(512)
	bee2.BignOidToDER(bee2.Vp2op(der), count, "1.2.112.0.2.0.34.101.31.81")
	c1 := bee2.SizeTarr_getitem(count, 0)
	fmt.Println(c1)

	sig := bee2.MemAlloc(64+32)
	err := bee2.BignSign2(bee2.Vp2op(sig), params, bee2.Vp2op(der), c1, 
		bee2.Vp2op(hash1), bee2.Vp2op(privkey), 0, 0)
	fmt.Println(err)
	bee2.HexFrom(bee2.Vp2bp(cbuf), sig, 64+32)
	sbuf = bee2.Cdata(cbuf, 64+32)
	fmt.Println(string(sbuf))

	err = bee2.BignVerify(params, bee2.Vp2op(der), c1, 
		bee2.Vp2op(hash1), bee2.Vp2op(sig), bee2.Vp2op(pubkey))
	fmt.Println(err)

	bee2.MemFree(privkey)
	bee2.MemFree(pubkey)
	bee2.MemFree(hash1)
	bee2.MemFree(sig)
	bee2.MemFree(der)
	bee2.Delete_sizeTarr(count)

}