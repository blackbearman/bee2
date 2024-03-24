library("bee2r")

setClass('_p_void', contains = 'ExternalReference')
setClass('_p_octet', contains = 'ExternalReference')
setClass('_p_size_t', contains = 'ExternalReference')
setClass('_p_SWIGCDATA', representation( ref = "character"))

################################################################################
## Test call function:  
# [1] 200
print(beltHash_keep())

################################################################################
## Test standard parameters
buf = memAlloc(128)

params = bign_params()
err = bignParamsStd(params, "1.2.112.0.2.0.34.101.45.3.1")
## Print int:      
# [1] 128
print(bign_params_l_get(params))

q = bign_params_q_get(params)
#rq = cdata(q, 32)  # binary data
printOctets <- function(q, buf, n) {
  hexFrom(vp2op(buf), op2vp(q), n)
  rq = cdata(buf, 2 * n)
  print(slot(rq,"ref"))
}

## Print octets:      
# [1] "07663D2699BF5A7EFC4DFB0DD68E5CD9FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF"
printOctets(q, buf, 32)

################################################################################
## Test pubkey 
privkey = memAlloc(64)
pubkey = memAlloc(128)

hexTo(privkey, "1F66B5B84B7339674533F0329C74F21834281FED0732429E0C79235FC273E269")
err = bignPubkeyCalc(vp2op(pubkey), params, vp2op(privkey))
printOctets(pubkey, buf, 64)
# [1] "BD1A5650179D79E03FCEE49D4C2BD5DDF54CE46D0CF11E4FF87BF7A890857FD0
#      7AC6A60361E8C8173491686D461B2826190C2EDA5909054A9AB84D2AB9D99A90"

################################################################################
hash1 = memAlloc(64)
hash = vp2op(hash1)
src = op2vp(beltH())
# Fix types by rewriting function 
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
printOctets(hash, buf, 64)
# [1] "ABEF9725D4C5A83597A367D14494CC2542F20F659DDFECC961A3EC550CBA8C75
#      002098EB5F550000502DFBEA5F550000002098EB5F550000D02FFBEA5F550000"

################################################################################
## Test DER
sig = memAlloc(64+32)
der = memAlloc(512)
count = new_sizeTarr(1)

sizeTarr_setitem(count, 0, 512)
bignOidToDER(vp2op(der), count, "1.2.112.0.2.0.34.101.31.81")
c1 = sizeTarr_getitem(count, 0)
print(c1)
# [1] 11

################################################################################
## Test signature
err = bignSign2(vp2op(sig), params, vp2op(der), c1, vp2op(hash1), vp2op(privkey), 0, 0)
print(err)

printOctets(sig, buf, 16*3)
# $ "19D32B7E01E25BAE4A70EB6BCA42602CCA6A13944451BCC5D4C54CFD8737619C
#    328B8A58FB9C68FD17D569F7D06495FB"
err = bignVerify(params, vp2op(der), c1, vp2op(hash1), vp2op(sig), vp2op(pubkey))
print(err)

################################################################################
# End
memFree(buf)
memFree(privkey)
memFree(pubkey)
memFree(hash1)
memFree(sig)
memFree(der)
delete_sizeTarr(count)
