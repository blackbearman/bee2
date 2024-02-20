library("bee2r")

setClass('_p_void', contains = 'ExternalReference')
setClass('_p_octet', contains = 'ExternalReference')
setClass('_p_size_t', contains = 'ExternalReference')
setClass('_p_SWIGCDATA', representation( ref = "character"))

################################################################################
buf = memAlloc(128)

params = bign_params()
err = bignParamsStd(params, "1.2.112.0.2.0.34.101.45.3.1")
print(bign_params_l_get(params))

q = bign_params_q_get(params)
#rq = cdata(q, 32)  # binary data
hexFrom(vp2op(buf), op2vp(q), 32)
rq = cdata(buf, 64)
print(slot(rq,"ref"))
# [1] "07663D2699BF5A7EFC4DFB0DD68E5CD9FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF"

#print(buf)
################################################################################
privkey = memAlloc(64)
pubkey = memAlloc(128)

hexTo(privkey, "1F66B5B84B7339674533F0329C74F21834281FED0732429E0C79235FC273E269")
err = bignPubkeyCalc(vp2op(pubkey), params, vp2op(privkey))
################################################################################
hash1 = memAlloc(64)
hash = vp2op(hash1)
src = op2vp(beltH())

`beltHash` = function(hash, src, count, .copy = FALSE)
{
  if (inherits(hash, "ExternalReference")) hash = slot(hash,"ref") 
  if (inherits(src, "ExternalReference")) src = slot(src,"ref")
  count = as.integer(count);
  
  if(length(count) > 1) {
    warning("using only the first element of count");
  };
  
  ;.Call('R_swig_beltHash', hash, src, count, as.logical(.copy), PACKAGE='bee2r');
  
}

attr(`beltHash`, 'returnType') = 'integer'
attr(`beltHash`, "inputTypes") = c('_p_octet', '_p_void', 'integer')
class(`beltHash`) = c("SWIGFunction", class('beltHash'))

err = beltHash(hash, src, 13)

################################################################################
sig = memAlloc(64+32)
der = memAlloc(512)
count = new_sizeTarr(1)

sizeTarr_setitem(count, 0, 512)
bignOidToDER(vp2op(der), count, "1.2.112.0.2.0.34.101.31.81")
c1 = sizeTarr_getitem(count, 0)
# c1 = 11

################################################################################
err = bignSign2(vp2op(sig), params, vp2op(der), c1, vp2op(hash1), vp2op(privkey), 0, 0)
print(err)
#sbuf = "0"*1000
hexFrom(vp2op(buf), sig, 16*3)
rq = cdata(buf, 16*3*2)
print(slot(rq,"ref"))
#[1] "19D32B7E01E25BAE4A70EB6BCA42602CCA6A13944451BCC5D4C54CFD8737619C328B8A58FB9C68FD17D569F7D06495FB"

err = bignVerify(params, vp2op(der), c1, vp2op(hash1), vp2op(sig), vp2op(pubkey))
print(err)

# end
memFree(buf)
memFree(privkey)
memFree(pubkey)
memFree(hash1)
memFree(sig)
memFree(der)
delete_sizeTarr(count)
