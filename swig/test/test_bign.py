from bee2py import vp2op, memAlloc, memFree
import bee2py

# begin
buf = memAlloc(128)
privkey = memAlloc(64)
pubkey = memAlloc(128)
hash1 = memAlloc(64)
sig = memAlloc(64+32)
der = memAlloc(512)
count = bee2py.new_sizeTarr(1)

sbuf = "0"*100
params = bee2py.bign_params()
err = bee2py.bignParamsStd(params, "1.2.112.0.2.0.34.101.45.3.1")
print(params.l)
sbuf = bee2py.hexFrom(sbuf, params.q, 32)
print(sbuf)

sbuf = "0"*1000
bee2py.hexTo(privkey, "1F66B5B84B7339674533F0329C74F21834281FED0732429E0C79235FC273E269")
bee2py.bignPubkeyCalc(vp2op(pubkey), params, vp2op(privkey))
sbuf = bee2py.hexFrom(sbuf, pubkey, 64)
print(sbuf)

	# // тест Г.6
	# if (beltHash(hash, beltH(), 13) != ERR_OK)
	# 	return FALSE;
	# if (bignSign2(sig, params, der, count, hash, privkey, 0, 0) 
	# 	!= ERR_OK)
	# 	return FALSE;
bee2py.beltHash(vp2op(hash1), bee2py.beltH(), 13)
sbuf = bee2py.hexFrom(sbuf, hash1, 64)
print(sbuf)

bee2py.sizeTarr_setitem(count, 0, 512)
#bee2py.bignParamsEnc(vp2op(der), count, params)
bee2py.bignOidToDER(vp2op(der), count, "1.2.112.0.2.0.34.101.31.81")
c1 = bee2py.sizeTarr_getitem(count, 0)
print(c1)
sbuf = "0"*1000
sbuf = bee2py.hexFrom(sbuf, der, c1)
print(sbuf)

err = bee2py.bignSign2(vp2op(sig), params, vp2op(der), c1, vp2op(hash1), vp2op(privkey), None, 0)
print(err)
sbuf = "0"*1000
sbuf = bee2py.hexFrom(sbuf, sig, 64+32)
print(sbuf)
err = bee2py.bignVerify(params, vp2op(der), c1, vp2op(hash1), vp2op(sig), vp2op(pubkey))
print(err)

# end
memFree(buf)
memFree(privkey)
memFree(pubkey)
memFree(hash1)
memFree(sig)
memFree(der)
bee2py.delete_sizeTarr(count)
